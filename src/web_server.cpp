#include "web_server.h"

#include <ArduinoJson.h>
#include <AsyncJson.h>
#include <StreamString.h>
#include <SD.h>
#include <FS.h>
#include <mbedtls/md.h>
#include <psram_allocator.h>
#include <MD5Builder.h>

#include "wifi_manager.h"
#include "config_manager.h"
#include "ui/ui_interface.h"

#include "operations.h"
#include "hardware.h"
#include "web/include/index.html.gz.h"
#include "web/include/login.html.gz.h"
#include "web/include/fs.html.gz.h"

enum class static_file_type
{
	array_zipped,
	sdcard,
};

typedef struct
{
	const char *path;
	const void *data;
	const uint32_t size;
	const char *media_type;
	const static_file_type type;
} static_files_map;

static const char JsonMediaType[] = "application/json";
static const char JsMediaType[] = "text/javascript";
static const char HtmlMediaType[] = "text/html";
static const char CssMediaType[] = "text/css";
static const char PngMediaType[] = "image/png";
static const char TextPlainMediaType[] = "text/plain";

static const char LoginUrl[] = "/login.html";
static const char IndexUrl[] = "/index.html";
static const char LogoUrl[] = "/media/logo.png";
static const char FaviconUrl[] = "/media/favicon.png";
static const char LogoutUrl[] = "/media/logout.png";
static const char SettingsUrl[] = "/media/settings.png";
static const char AllJsUrl[] = "/js/s.js";
static const char MdbCssUrl[] = "/css/mdb.min.css";

static const char FSListUrl[] = "/fs.html";

static const char MD5Header[] = "md5";
static const char CacheControlHeader[] = "Cache-Control";
static const char CookieHeader[] = "Cookie";
static const char AuthCookieName[] = "ESPSESSIONID=";

const static static_files_map static_files[] = {
	{FSListUrl, fs_html_gz, fs_html_gz_len, HtmlMediaType, static_file_type::array_zipped},
	{IndexUrl, index_html_gz, index_html_gz_len, HtmlMediaType, static_file_type::array_zipped},
	{LoginUrl, login_html_gz, login_html_gz_len, HtmlMediaType, static_file_type::array_zipped},
	{LogoUrl, "/web/logo.png", 0, PngMediaType, static_file_type::sdcard},
	{FaviconUrl, "/web/logo.png", 0, PngMediaType, static_file_type::sdcard},
	{AllJsUrl, "/web/s.js", 0, JsMediaType, static_file_type::sdcard},
	{MdbCssUrl, "/web/mdb.min.css", 0, CssMediaType, static_file_type::sdcard},
};

web_server web_server::instance;

String create_hash(const String &user, const String &password, const String &ipAddress)
{
	byte hmacResult[20];
	mbedtls_md_context_t ctx;
	mbedtls_md_type_t md_type = MBEDTLS_MD_SHA1;

	mbedtls_md_init(&ctx);
	mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 1);
	mbedtls_md_hmac_update(&ctx, reinterpret_cast<const unsigned char *>(user.c_str()), user.length());
	mbedtls_md_hmac_update(&ctx, reinterpret_cast<const unsigned char *>(password.c_str()), password.length());
	mbedtls_md_hmac_update(&ctx, reinterpret_cast<const unsigned char *>(ipAddress.c_str()), ipAddress.length());
	mbedtls_md_hmac_finish(&ctx, hmacResult);
	mbedtls_md_free(&ctx);

	StreamString stream;
	for (auto i = 0; i < 20; i++)
	{
		stream += String(hmacResult[i], HEX);
	}
	return stream;
}

void web_server::begin()
{
	log_i("WebServer Starting");
	events.onConnect(std::bind(&web_server::on_event_connect, this, std::placeholders::_1));
	events.setFilter(std::bind(&web_server::filter_events, this, std::placeholders::_1));
	http_server.addHandler(&events);
	http_server.begin();
	server_routing();
	log_i("WebServer Started");

	for (auto i = 0; i < total_sensors; i++)
	{
		const auto id = static_cast<sensor_id_index>(i);
		hardware::instance.get_sensor(id).add_callback([id, this]
													   { notify_sensor_change(id); });
	}
}

bool web_server::manage_security(AsyncWebServerRequest *request)
{
	if (!is_authenticated(request))
	{
		log_w("Auth Failed");
		request->send(401, JsonMediaType, F("{\"msg\": \"Not-authenticated\"}"));
		return false;
	}
	return true;
}

bool web_server::filter_events(AsyncWebServerRequest *request)
{
	if (!is_authenticated(request))
	{
		log_w("Dropping events request");
		return false;
	}
	return true;
}

void web_server::server_routing()
{
	// form calls
	http_server.on(("/login.handler"), HTTP_POST, handle_login);
	http_server.on(("/logout.handler"), HTTP_POST, handle_logout);
	http_server.on(("/wifiupdate.handler"), HTTP_POST, wifi_update);

	http_server.on(("/othersettings.update.handler"), HTTP_POST, other_settings_update);
	http_server.on(("/weblogin.update.handler"), HTTP_POST, web_login_update);

	// ajax form call
	http_server.on(("/factory.reset.handler"), HTTP_POST, factory_reset);
	http_server.on(("/firmware.update.handler"), HTTP_POST, reboot_on_upload_complete, firmware_update_upload);
	http_server.on(("/setting.restore.handler"), HTTP_POST, reboot_on_upload_complete,
				   std::bind(&web_server::restore_configuration_upload, this,
							 std::placeholders::_1, std::placeholders::_2, std::placeholders::_3,
							 std::placeholders::_4, std::placeholders::_5, std::placeholders::_6));

	http_server.on(("/restart.handler"), HTTP_POST, restart_device);

	// json ajax calls
	http_server.on(("/api/sensor/get"), HTTP_GET, sensor_get);
	http_server.on(("/api/wifi/get"), HTTP_GET, wifi_get);
	http_server.on(("/api/information/get"), HTTP_GET, information_get);
	http_server.on(("/api/config/get"), HTTP_GET, config_get);

	// fs ajax
	http_server.on("/fs/list", HTTP_GET, handle_dir_list);
	http_server.on("/fs/download", HTTP_GET, handle_fs_download);
	http_server.on("/fs/upload", HTTP_POST, handle_file_upload_complete,
				   std::bind(&web_server::handle_file_upload, this,
							 std::placeholders::_1, std::placeholders::_2, std::placeholders::_3,
							 std::placeholders::_4, std::placeholders::_5, std::placeholders::_6));
	http_server.on("/fs/delete", HTTP_POST, handle_fs_delete);
	http_server.on("/fs/rename", HTTP_POST, handle_fs_rename);
	http_server.onNotFound(handle_file_read);
}

void web_server::on_event_connect(AsyncEventSourceClient *client)
{
	if (client->lastId())
	{
		log_i("Events client reconnect");
	}
	else
	{
		log_i("Events client first time");

		// send all the events
		for (auto i = 0; i < total_sensors; i++)
		{
			notify_sensor_change(static_cast<sensor_id_index>(i));
		}
	}
}

void web_server::wifi_get(AsyncWebServerRequest *request)
{
	log_i("/api/wifi/get");
	if (!manage_security(request))
	{
		return;
	}

	auto response = new AsyncJsonResponse(false, 256);
	auto jsonBuffer = response->getRoot();

	jsonBuffer[F("captivePortal")] = wifi_manager::instance.is_captive_portal();
	jsonBuffer[F("ssid")] = config::instance.data.get_wifi_ssid();
	response->setLength();
	request->send(response);
}

void web_server::wifi_update(AsyncWebServerRequest *request)
{
	const auto SsidParameter = F("ssid");
	const auto PasswordParameter = F("wifipassword");

	log_i("Wifi Update");

	if (!manage_security(request))
	{
		return;
	}

	if (request->hasArg(SsidParameter) && request->hasArg(PasswordParameter))
	{
		wifi_manager::instance.set_new_wifi(request->arg(SsidParameter), request->arg(PasswordParameter));
		redirect_to_root(request);
		return;
	}
	else
	{
		handle_error(request, F("Required parameters not provided"), 400);
	}
}

template <class Array, class K, class T>
void web_server::add_key_value_object(Array &array, const K &key, const T &value)
{
	auto j1 = array.createNestedObject();
	j1[F("key")] = key;
	j1[F("value")] = value;
}

void web_server::information_get(AsyncWebServerRequest *request)
{
	log_d("/api/information/get");
	if (!manage_security(request))
	{
		return;
	}

	auto response = new AsyncJsonResponse(true, 1024);
	auto arr = response->getRoot();

	const auto data = hardware::instance.get_information_table(ui_interface::information_type::system);

	for (auto &&[key, value] : data)
	{
		add_key_value_object(arr, key, value);
	}

	response->setLength();
	request->send(response);
}

void web_server::config_get(AsyncWebServerRequest *request)
{
	log_w("/api/information/get");
	if (!manage_security(request))
	{
		return;
	}
	const auto json = config::instance.get_all_config_as_json();
	request->send(200, FPSTR(JsonMediaType), json);
}

template <class V, class T>
void web_server::add_to_json_doc(V &doc, T id, float value)
{
	if (!isnan(value))
	{
		doc[id] = serialized(String(value, 2));
	}
}

void web_server::sensor_get(AsyncWebServerRequest *request)
{
	log_d("/api/sensor/get");
	if (!manage_security(request))
	{
		return;
	}
	auto response = new AsyncJsonResponse(false, 256);
	auto doc = response->getRoot();
	response->setLength();
	request->send(response);
}

// Check if header is present and correct
bool web_server::is_authenticated(AsyncWebServerRequest *request)
{
	log_v("Checking if authenticated");
	if (request->hasHeader(FPSTR(CookieHeader)))
	{
		const String cookie = request->header(FPSTR(CookieHeader));
		log_v("Found cookie:%s", cookie.c_str());

		const String token = create_hash(config::instance.data.get_web_user_name(),
										 config::instance.data.get_web_password(),
										 request->client()->remoteIP().toString());

		if (cookie.indexOf(String(AuthCookieName) + token) != -1)
		{
			log_v("Authentication Successful");
			return true;
		}
	}
	log_d("Authentication Failed");
	return false;
}

void web_server::handle_login(AsyncWebServerRequest *request)
{
	const auto UserNameParameter = F("username");
	const auto PasswordParameter = F("password");

	log_i("Handle login");
	String msg;
	if (request->hasHeader(FPSTR(CookieHeader)))
	{
		// Print cookies
		log_v("Found cookie: %s", request->header(FPSTR(CookieHeader)).c_str());
	}

	if (request->hasArg(UserNameParameter) && request->hasArg(PasswordParameter))
	{
		const auto user = config::instance.data.get_web_user_name();
		const auto password = config::instance.data.get_web_password();
		if (request->arg(UserNameParameter).equalsIgnoreCase(user) &&
			request->arg(PasswordParameter).equalsConstantTime(password))
		{
			log_w("User/Password correct");
			auto response = request->beginResponse(301); // Sends 301 redirect

			response->addHeader(F("Location"), F("/"));
			response->addHeader(FPSTR(CacheControlHeader), F("no-cache"));

			const String token = create_hash(user, password, request->client()->remoteIP().toString());
			log_d("Token:%s", token.c_str());
			response->addHeader(F("Set-Cookie"), String(AuthCookieName) + token);

			request->send(response);
			log_i("Log in Successful");
			return;
		}

		msg = F("Wrong username/password! Try again.");
		log_w("Log in Failed");
		auto response = request->beginResponse(301); // Sends 301 redirect

		response->addHeader(F("Location"), String(F("/login.html?msg=")) + msg);
		response->addHeader(FPSTR(CacheControlHeader), F("no-cache"));
		request->send(response);
		return;
	}
	else
	{
		handle_error(request, F("Login Parameter not provided"), 400);
	}
}

/**
 * Manage logout (simply remove correct token and redirect to login form)
 */
void web_server::handle_logout(AsyncWebServerRequest *request)
{
	log_i("Disconnection");
	AsyncWebServerResponse *response = request->beginResponse(301); // Sends 301 redirect
	response->addHeader("Location", "/login.html?msg=User disconnected");
	response->addHeader("CacheControlHeader", "no-cache");
	response->addHeader("Set-Cookie", "ESPSESSIONID=0");
	request->send(response);
	return;
}

void web_server::web_login_update(AsyncWebServerRequest *request)
{
	const auto webUserName = "webUserName";
	const auto webPassword = "webPassword";

	log_i("web login Update");

	if (!manage_security(request))
	{
		return;
	}

	if (request->hasArg(webUserName) && request->hasArg("webPassword"))
	{
		log_i("Updating web username/password");
		config::instance.data.set_web_password(request->arg(webUserName));
		config::instance.data.set_wifi_password(request->arg(webPassword));
	}
	else
	{
		handle_error(request, F("Correct Parameters not provided"), 400);
	}

	config::instance.save();
	redirect_to_root(request);
}

void web_server::other_settings_update(AsyncWebServerRequest *request)
{
	const auto hostName = F("hostName");
	const auto ntpServer = F("ntpServer");
	const auto ntpServerRefreshInterval = F("ntpServerRefreshInterval");
	const auto timezone = F("timezone");
	const auto autoScreenBrightness = "autoScreenBrightness";
	const auto screenBrightness = "screenBrightness";

	log_i("config Update");

	if (!manage_security(request))
	{
		return;
	}

	if (request->hasArg(hostName))
	{
		config::instance.data.set_host_name(request->arg(hostName));
	}

	if (request->hasArg(ntpServer))
	{
		config::instance.data.set_ntp_server(request->arg(ntpServer));
	}

	if (request->hasArg(ntpServerRefreshInterval))
	{
		config::instance.data.set_ntp_server_refresh_interval(request->arg(ntpServerRefreshInterval).toInt() * 1000);
	}

	if (request->hasArg(timezone))
	{
		config::instance.data.set_timezone(static_cast<TimeZoneSupported>(request->arg(timezone).toInt()));
	}

	if (!request->hasArg(autoScreenBrightness))
	{
		config::instance.data.set_manual_screen_brightness(request->arg(screenBrightness).toInt());
	}
	else
	{
		config::instance.data.set_manual_screen_brightness(std::nullopt);
	}

	config::instance.save();
	redirect_to_root(request);
}

void web_server::restart_device(AsyncWebServerRequest *request)
{
	log_i("restart");

	if (!manage_security(request))
	{
		return;
	}

	request->send(200);
	operations::instance.reboot();
}

void web_server::factory_reset(AsyncWebServerRequest *request)
{
	log_i("factoryReset");

	if (!manage_security(request))
	{
		return;
	}

	request->send(200);
	operations::instance.factory_reset();
}

void web_server::reboot_on_upload_complete(AsyncWebServerRequest *request)
{
	log_i("reboot");

	if (!manage_security(request))
	{
		return;
	}

	request->send(200);
	operations::instance.reboot();
}

void web_server::handle_file_read(AsyncWebServerRequest *request)
{
	auto path = request->url();
	log_d("handleFileRead: %s", path.c_str());

	if (path.endsWith(F("/")) || path.isEmpty())
	{
		log_d("Redirecting to index page");
		path = FPSTR(IndexUrl);
	}

	const bool worksWithoutAuth = path.startsWith(F("/media/")) ||
								  path.startsWith(F("/js/")) ||
								  path.startsWith(F("/css/")) ||
								  path.startsWith(F("/font/")) ||
								  path.equalsIgnoreCase(FPSTR(LoginUrl));

	if (!worksWithoutAuth && !is_authenticated(request))
	{
		log_d("Redirecting to login page");
		path = String(LoginUrl);
	}

	for (size_t i = 0; i < sizeof(static_files) / sizeof(static_files[0]); i++)
	{
		const auto entryPath = static_files[i].path;
		if (path.equalsIgnoreCase(entryPath))
		{
			auto &&static_file = static_files[i];
			const String mediaType(static_file.media_type);
			AsyncWebServerResponse *response = nullptr;

			switch (static_file.type)
			{
			case static_file_type::array_zipped:
				response = request->beginResponse_P(200,
													mediaType,
													reinterpret_cast<const uint8_t *>(static_file.data),
													static_file.size);
				response->addHeader("Content-Encoding", "gzip");

				break;
			case static_file_type::sdcard:
				response = request->beginResponse(SD,
												  reinterpret_cast<const char *>(static_file.data),
												  mediaType);
			}

			if (response)
			{
				if (worksWithoutAuth)
				{
					response->addHeader(CacheControlHeader, "public, max-age=31536000");
				}

				request->send(response);
				log_d("Served path:%s mimeType: %s", path.c_str(), mediaType.c_str());
				return;
			}
			else
			{
				log_w("File not found path:%s mimeType: %s", path.c_str(), mediaType.c_str());
			}
		}
	}

	handle_not_found(request);
}

/** Redirect to captive portal if we got a request for another domain.
 * Return true in that case so the page handler do not try to handle the request again. */
bool web_server::is_captive_portal_request(AsyncWebServerRequest *request)
{
	if (!is_ip(request->host()))
	{
		log_i("Request redirected to captive portal");
		AsyncWebServerResponse *response = request->beginResponse(302, String(TextPlainMediaType), String());
		response->addHeader(F("Location"), String("http://") + to_string_ip(request->client()->localIP()));
		request->send(response);
		return true;
	}
	return false;
}

void web_server::handle_not_found(AsyncWebServerRequest *request)
{
	if (is_captive_portal_request(request))
	{
		// if captive portal redirect instead of displaying the error page
		return;
	}

	String message = F("File Not Found\n\n");
	message += F("URI: ");
	message += request->url();
	message += F("\nMethod: ");
	message += (request->method() == HTTP_GET) ? F("GET") : F("POST");
	message += F("\nArguments: ");
	message += request->args();
	message += F("\n");

	for (unsigned int i = 0; i < request->args(); i++)
	{
		message += String(F(" ")) + request->argName(i) + F(": ") + request->arg(i) + "\n";
	}

	handle_error(request, message, 404);
}

// is this an IP?
bool web_server::is_ip(const String &str)
{
	for (unsigned int i = 0; i < str.length(); i++)
	{
		int c = str.charAt(i);
		if (c != '.' && (c < '0' || c > '9'))
		{
			return false;
		}
	}
	return true;
}

String web_server::to_string_ip(const IPAddress &ip)
{
	return ip.toString();
}

void web_server::redirect_to_root(AsyncWebServerRequest *request)
{
	AsyncWebServerResponse *response = request->beginResponse(301); // Sends 301 redirect
	response->addHeader(F("Location"), F("/"));
	request->send(response);
}

void web_server::firmware_update_upload(AsyncWebServerRequest *request,
										const String &filename,
										size_t index,
										uint8_t *data,
										size_t len,
										bool final)
{
	log_d("firmwareUpdateUpload");

	if (!manage_security(request))
	{
		return;
	}

	String error;
	if (!index)
	{
		String md5;

		if (request->hasHeader(MD5Header))
		{
			md5 = request->getHeader(MD5Header)->value();
		}

		log_i("Expected MD5:%s", md5.c_str());

		if (md5.length() != 32)
		{
			handle_error(request, F("MD5 parameter invalid. Check file exists."), 500);
			return;
		}

		if (operations::instance.start_update(request->contentLength(), md5, error))
		{
			// success, let's make sure we end the update if the client hangs up
			request->onDisconnect(handle_early_update_disconnect);
		}
		else
		{
			handle_error(request, error, 500);
			return;
		}
	}

	if (operations::instance.is_update_in_progress())
	{
		if (!operations::instance.write_update(data, len, error))
		{
			handle_error(request, error, 500);
		}

		if (final)
		{
			if (!operations::instance.end_update(error))
			{
				handle_error(request, error, 500);
			}
		}
	}
}

void web_server::restore_configuration_upload(AsyncWebServerRequest *request,
											  const String &filename,
											  size_t index,
											  uint8_t *data,
											  size_t len,
											  bool final)
{
	log_i("restoreConfigurationUpload");

	if (!manage_security(request))
	{
		return;
	}

	String error;
	if (!index)
	{
		restore_config_data = std::make_unique<std::vector<uint8_t>>();
	}

	for (size_t i = 0; i < len; i++)
	{
		restore_config_data->push_back(data[i]);
	}

	if (final)
	{
		String md5;
		if (request->hasHeader(FPSTR(MD5Header)))
		{
			md5 = request->getHeader(FPSTR(MD5Header))->value();
		}

		log_d("Expected MD5:%s", md5.c_str());

		if (md5.length() != 32)
		{
			handle_error(request, F("MD5 parameter invalid. Check file exists."), 500);
			return;
		}

		if (!config::instance.restore_all_config_as_json(*web_server::instance.restore_config_data, md5))
		{
			handle_error(request, F("Restore Failed"), 500);
			return;
		}
	}
}

void web_server::handle_error(AsyncWebServerRequest *request, const String &message, int code)
{
	if (!message.isEmpty())
	{
		log_e("%s", message.c_str());
	}
	AsyncWebServerResponse *response = request->beginResponse(code, TextPlainMediaType, message);
	response->addHeader(FPSTR(CacheControlHeader), F("no-cache, no-store, must-revalidate"));
	response->addHeader(F("Pragma"), F("no-cache"));
	response->addHeader(F("Expires"), F("-1"));
	request->send(response);
}

void web_server::handle_early_update_disconnect()
{
	operations::instance.abort_update();
}

void web_server::notify_sensor_change(sensor_id_index id)
{
	if (events.count())
	{
		const auto &sensor = hardware::instance.get_sensor(id);
		const auto value = sensor.get_value();
		const String value_str = value.has_value() ? String(value.value(), 10) : String("-");

		BasicJsonDocument<psram::psram_json_allocator> json_document(128);

		auto &&definition = get_sensor_definition(id);
		json_document["value"] = value_str;
		json_document["unit"] = definition.get_unit();
		json_document["type"] = definition.get_name();
		json_document["level"] = definition.calculate_level(value.value_or(0));

		String json;
		serializeJson(json_document, json);
		events.send(json.c_str(), "sensor", millis());
	}
}

void web_server::handle_dir_list(AsyncWebServerRequest *request)
{
	const auto dir_param = "dir";

	log_d("/fs/list");
	if (!manage_security(request))
	{
		return;
	}

	if (!request->hasArg(dir_param))
	{
		handle_error(request, "Bad Arguments", 500);
		return;
	}

	const auto path = request->arg(dir_param);

	auto dir = SD.open(path);

	if (!dir)
	{
		handle_error(request, "Failed to open directory:" + path, 500);
		return;
	}

	if (!dir.isDirectory())
	{
		handle_error(request, "Not a directory:" + path, 500);
		dir.close();
		return;
	}

	auto response = new AsyncJsonResponse(false, 1024 * 16);
	auto root = response->getRoot();
	auto array = root.createNestedArray("data");
	auto entry = dir.openNextFile();

	const auto path_with_slash = path.endsWith("/") ? path : path + "/";
	while (entry)
	{
		auto nested_entry = array.createNestedObject();
		const auto name = String(entry.name());
		nested_entry["path"] = path_with_slash + name;
		nested_entry["isDir"] = entry.isDirectory();
		nested_entry["name"] = name;
		nested_entry["size"] = entry.size();
		nested_entry["lastModified"] = entry.getLastWrite();

		entry.close();
		entry = dir.openNextFile();
	}

	dir.close();

	response->setLength();
	request->send(response);
}

void web_server::handle_fs_download(AsyncWebServerRequest *request)
{
	const auto path_param = "path";

	log_d("/fs/download");
	if (!manage_security(request))
	{
		return;
	}

	if (!request->hasArg(path_param))
	{
		handle_error(request, "Bad Arguments", 500);
		return;
	}

	const auto path = request->arg(path_param);

	auto file = SD.open(path);

	if (!file)
	{
		handle_error(request, "Failed to open file:" + path, 500);
		return;
	}

	if (file.isDirectory())
	{
		handle_error(request, "Not a file:" + path, 500);
		file.close();
		return;
	}

	const bool download = request->hasArg("download");

	const auto contentType = download ? "application/octet-stream" : get_content_type(path);
	AsyncWebServerResponse *response = request->beginResponse(file, path, contentType, download);
	request->send(response);
}

void web_server::handle_fs_delete(AsyncWebServerRequest *request)
{
	const auto path_param = "deleteFilePath";

	log_d("/fs/delete");
	if (!manage_security(request))
	{
		return;
	}

	if (!request->hasArg(path_param))
	{
		handle_error(request, "Bad Arguments", 500);
		return;
	}

	const auto path = request->arg(path_param);

	log_i("Deleting %s", path.c_str());
	if (!SD.remove(path))
	{
		handle_error(request, "Failed to delete " + path, 500);
		return;
	}

	request->send(200);
}

void web_server::handle_fs_rename(AsyncWebServerRequest *request)
{
	const auto path_original_path = "oldPath";
	const auto path_dest_path = "newPath";

	log_d("/fs/rename");
	if (!manage_security(request))
	{
		return;
	}

	if (!request->hasArg(path_original_path) || !request->hasArg(path_dest_path))
	{
		handle_error(request, "Bad Arguments", 500);
		return;
	}

	const auto original_path = request->arg(path_original_path);
	const auto new_path = request->arg(path_dest_path);

	log_i("Renaming %s to %s", original_path.c_str(), new_path.c_str());
	if (!SD.rename(original_path, new_path))
	{
		handle_error(request, "Failed to rename to " + new_path, 500);
		return;
	}

	request->send(200);
}

void web_server::handle_file_upload(AsyncWebServerRequest *request,
									const String &filename,
									size_t index,
									uint8_t *data,
									size_t len,
									bool final)
{
	log_i("/fs/upload");

	if (!manage_security(request))
	{
		return;
	}

	const auto uploadDirHeader = "uploadDir";
	if (!index)
	{
		String uploadDir;
		if (request->hasHeader(uploadDirHeader))
		{
			const auto dir = request->getHeader(uploadDirHeader)->value();
			const auto full_path = join_path(dir, filename + ".tmp");

			request->_tempFile = SD.open(full_path, "w+", true);
			if (!request->_tempFile)
			{
				handle_error(request, "Failed to open the file", 500);
				return;
			}
		}
		else
		{
			handle_error(request, "Upload dir not specified", 500);
			return;
		}
	}

	if (request->_tempFile)
	{
		if (!request->_tempFile.seek(index))
		{
			handle_error(request, "Failed to seek the file", 500);
			return;
		}
		if (request->_tempFile.write(data, len) != len)
		{
			const auto error = request->_tempFile.getWriteError();
			handle_error(request, String("Failed to write to the file with error: ") + String(error, 10), 500);
			return;
		}
	}
	else
	{
		handle_error(request, "No open file for upload", 500);
		return;
	}

	if (final)
	{
		String md5;
		if (request->hasHeader(MD5Header))
		{
			md5 = request->getHeader(MD5Header)->value();
		}

		log_i("Expected MD5:%s", md5.c_str());

		if (md5.length() != 32)
		{
			handle_error(request, "MD5 parameter invalid. Check file exists", 500);
			return;
		}

		request->_tempFile.close();

		const auto dir = request->getHeader(uploadDirHeader)->value();
		const auto tmp_full_path = join_path(dir, filename + ".tmp");

		// calculate hash of written file
		md5.toUpperCase();
		auto disk_md5 = get_file_md5(tmp_full_path);
		disk_md5.toUpperCase();

		if (md5 != disk_md5)
		{
			handle_error(request, "Md5 hash of written file does not match. Found: " + disk_md5, 500);
			SD.remove(tmp_full_path);
			return;
		}

		const auto full_path = join_path(dir, filename);
		if (!SD.rename(tmp_full_path, full_path))
		{
			handle_error(request, "Failed from rename temp file failed", 500);
			return;
		}
	}
}

String web_server::get_file_md5(const String &path)
{
	auto file = SD.open(path);
	MD5Builder hashBuilder;
	hashBuilder.begin();

	file.seek(0);
	hashBuilder.addStream(file, file.size());
	hashBuilder.calculate();
	file.close();

	return hashBuilder.toString();
}

void web_server::handle_file_upload_complete(AsyncWebServerRequest *request)
{
	log_i("file upload complete");
	if (!manage_security(request))
	{
		return;
	}

	request->send(200);
}

const char *web_server::get_content_type(const String &filename)
{
	if (filename.endsWith(".htm"))
		return "text/html";
	else if (filename.endsWith(".html"))
		return "text/html";
	else if (filename.endsWith(".css"))
		return "text/css";
	else if (filename.endsWith(".js"))
		return "application/javascript";
	else if (filename.endsWith(".json"))
		return "application/json";
	else if (filename.endsWith(".png"))
		return "image/png";
	else if (filename.endsWith(".gif"))
		return "image/gif";
	else if (filename.endsWith(".jpg"))
		return "image/jpeg";
	else if (filename.endsWith(".ico"))
		return "image/x-icon";
	else if (filename.endsWith(".xml"))
		return "text/xml";
	else if (filename.endsWith(".pdf"))
		return "application/x-pdf";
	else if (filename.endsWith(".zip"))
		return "application/x-zip";
	else if (filename.endsWith(".gz"))
		return "application/x-gzip";
	return "text/plain";
}

String web_server::join_path(const String &part1, const String &part2)
{
	return part1 + (part1.endsWith("/") ? "" : "/") + part2;
}
