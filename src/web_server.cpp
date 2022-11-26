#include "web_server.h"

#include <ArduinoJson.h>
#include <AsyncJson.h>
#include <StreamString.h>
#include <mbedtls/md.h>

#include "wifi_manager.h"
#include "config_manager.h"
#include "ui/ui_interface.h"
		 
#include "operations.h"
#include "hardware.h"
#include "web.h"

typedef struct
{
	const char *Path;
	const unsigned char *Data;
	const uint32_t Size;
	const char *MediaType;
	const uint8_t Zipped;
} StaticFilesMap;

static const char JsonMediaType[] PROGMEM = "application/json";
static const char JsMediaType[] PROGMEM = "text/javascript";
static const char HtmlMediaType[] PROGMEM = "text/html";
static const char CssMediaType[] PROGMEM = "text/css";
static const char PngMediaType[] PROGMEM = "image/png";
static const char TextPlainMediaType[] PROGMEM = "text/plain";

static const char LoginUrl[] PROGMEM = "/login.html";
static const char IndexUrl[] PROGMEM = "/index.html";
static const char LogoUrl[] PROGMEM = "/media/logo.png";
static const char FaviconUrl[] PROGMEM = "/media/favicon.png";
static const char LogoutUrl[] PROGMEM = "/media/logout.png";
static const char SettingsUrl[] PROGMEM = "/media/settings.png";
static const char AllJsUrl[] PROGMEM = "/js/s.js";
// static const char JQueryJsUrl[] PROGMEM = "/js/jquery.min.js";
// static const char MdbJsUrl[] PROGMEM = "/js/mdb.min.js";
static const char MdbCssUrl[] PROGMEM = "/css/mdb.min.css";

static const char MD5Header[] PROGMEM = "md5";
static const char CacheControlHeader[] PROGMEM = "Cache-Control";
static const char CookieHeader[] PROGMEM = "Cookie";
static const char AuthCookieName[] PROGMEM = "ESPSESSIONID=";

const static StaticFilesMap staticFilesMap[] PROGMEM = {
	{IndexUrl, index_html_gz, index_html_gz_len, HtmlMediaType, true},
	{LoginUrl, login_html_gz, login_html_gz_len, HtmlMediaType, true},
	{LogoUrl, logo_png, logo_png_len, PngMediaType, false},
	{FaviconUrl, logo_png, logo_png_len, PngMediaType, false},
	{AllJsUrl, s_js_gz, s_js_gz_len, JsMediaType, true},
	//{JQueryJsUrl, jquery_min_js_gz, jquery_min_js_gz_len, JsMediaType, true},
	//{MdbJsUrl, mdb_min_js_gz, mdb_min_js_gz_len, JsMediaType, true},
	{MdbCssUrl, mdb_min_css_gz, mdb_min_css_gz_len, CssMediaType, true},
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
	events.onConnect(std::bind(&web_server::onEventConnect, this, std::placeholders::_1));
	events.setFilter(std::bind(&web_server::filterEvents, this, std::placeholders::_1));
	http_server.addHandler(&events);
	http_server.begin();
	serverRouting();
	log_i("WebServer Started");

	// hardware::instance.temperatureChangeCallback.addConfigSaveCallback(std::bind(&WebServer::notifySensorChange, this));
}

bool web_server::manageSecurity(AsyncWebServerRequest *request)
{
	if (!isAuthenticated(request))
	{
		log_w("Auth Failed");
		request->send(401, FPSTR(JsonMediaType), F("{\"msg\": \"Not-authenticated\"}"));
		return false;
	}
	return true;
}

bool web_server::filterEvents(AsyncWebServerRequest *request)
{
	if (!isAuthenticated(request))
	{
		log_w("Dropping events request");
		return false;
	}
	return true;
}

void web_server::serverRouting()
{
	// form calls
	http_server.on(("/login.handler"), HTTP_POST, handleLogin);
	http_server.on(("/logout.handler"), HTTP_POST, handleLogout);
	http_server.on(("/wifiupdate.handler"), HTTP_POST, wifiUpdate);

	http_server.on(("/othersettings.update.handler"), HTTP_POST, otherSettingsUpdate);
	http_server.on(("/weblogin.update.handler"), HTTP_POST, webLoginUpdate);

	// ajax form call
	http_server.on(("/factory.reset.handler"), HTTP_POST, factoryReset);
	http_server.on(("/firmware.update.handler"), HTTP_POST, rebootOnUploadComplete, firmwareUpdateUpload);
	http_server.on(("/setting.restore.handler"), HTTP_POST, rebootOnUploadComplete, restoreConfigurationUpload);
	http_server.on(("/restart.handler"), HTTP_POST, restartDevice);

	// json ajax calls
	http_server.on(("/api/sensor/get"), HTTP_GET, sensorGet);
	http_server.on(("/api/wifi/get"), HTTP_GET, wifiGet);
	http_server.on(("/api/information/get"), HTTP_GET, informationGet);
	http_server.on(("/api/config/get"), HTTP_GET, configGet);

	http_server.onNotFound(handleFileRead);
}

void web_server::onEventConnect(AsyncEventSourceClient *client)
{
	if (client->lastId())
	{
		log_i("Events client reconnect");
	}
	else
	{
		log_i("Events client first time");
		// send all the events
		notifySensorChange();
	}
}

void web_server::wifiGet(AsyncWebServerRequest *request)
{
	log_i("/api/wifi/get");
	if (!manageSecurity(request))
	{
		return;
	}

	auto response = new AsyncJsonResponse(false, 256);
	auto jsonBuffer = response->getRoot();

	jsonBuffer[F("captivePortal")] = wifi_manager::instance.isCaptivePortal();
	jsonBuffer[F("ssid")] = wifi_manager::SSID();

	response->setLength();
	request->send(response);
}

void web_server::wifiUpdate(AsyncWebServerRequest *request)
{
	const auto SsidParameter = F("ssid");
	const auto PasswordParameter = F("wifipassword");

	log_i("Wifi Update");

	if (!manageSecurity(request))
	{
		return;
	}

	if (request->hasArg(SsidParameter) && request->hasArg(PasswordParameter))
	{
		wifi_manager::instance.set_new_wifi(request->arg(SsidParameter), request->arg(PasswordParameter));
		redirectToRoot(request);
		return;
	}
	else
	{
		handleError(request, F("Required parameters not provided"), 400);
	}
}

template <class Array, class K, class T>
void web_server::addKeyValueObject(Array &array, const K &key, const T &value)
{
	auto j1 = array.createNestedObject();
	j1[F("key")] = key;
	j1[F("value")] = value;
}

void web_server::informationGet(AsyncWebServerRequest *request)
{
	log_d("/api/information/get");
	if (!manageSecurity(request))
	{
		return;
	}

	// const auto maxFreeHeapSize = ESP.getMaxFreeBlockSize() / 1024;
	// const auto freeHeap = ESP.getFreeHeap() / 1024;

	auto response = new AsyncJsonResponse(true, 1024);
	auto arr = response->getRoot();

	const auto data = ui_interface::instance.get_information_table();

	for (auto &&[key, value] : data)
	{
		addKeyValueObject(arr, key, value);
	}

	// addKeyValueObject(arr, F("Version"), VERSION);
	// addKeyValueObject(arr, F("Uptime"), GetUptime());
	// addKeyValueObject(arr, F("AP SSID"), WiFi.SSID());
	// addKeyValueObject(arr, F("AP Signal Strength"), WiFi.RSSI());
	// addKeyValueObject(arr, F("Mac Address"), WiFi.softAPmacAddress());

	// addKeyValueObject(arr, F("Reset Info"), ESP.getResetInfo());
	// addKeyValueObject(arr, F("CPU Frequency (MHz)"), system_get_cpu_freq());

	// addKeyValueObject(arr, F("Max Block Free Size (KB)"), maxFreeHeapSize);
	// addKeyValueObject(arr, F("Free Heap (KB)"), freeHeap);

	// FSInfo fsInfo;
	// LittleFS.info(fsInfo);

	// addKeyValueObject(arr, F("Filesystem Total Size (KB)"), fsInfo.totalBytes / 1024);
	// addKeyValueObject(arr, F("Filesystem Free Size (KB)"), (fsInfo.totalBytes - fsInfo.usedBytes) / 1024);

	response->setLength();
	request->send(response);
}

void web_server::configGet(AsyncWebServerRequest *request)
{
	log_w("/api/information/get");
	if (!manageSecurity(request))
	{
		return;
	}
	const auto json = config::instance.getAllConfigAsJson();
	request->send(200, FPSTR(JsonMediaType), json);
}

template <class V, class T>
void web_server::addToJsonDoc(V &doc, T id, float value)
{
	if (!isnan(value))
	{
		doc[id] = serialized(String(value, 2));
	}
}

void web_server::sensorGet(AsyncWebServerRequest *request)
{
	log_d("/api/sensor/get");
	if (!manageSecurity(request))
	{
		return;
	}
	auto response = new AsyncJsonResponse(false, 256);
	auto doc = response->getRoot();

	// addToJsonDoc(doc, F("lux"), hardware::instance.getLux());
	response->setLength();
	request->send(response);
}

// Check if header is present and correct
bool web_server::isAuthenticated(AsyncWebServerRequest *request)
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

void web_server::handleLogin(AsyncWebServerRequest *request)
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
		handleError(request, F("Login Parameter not provided"), 400);
	}
}

/**
 * Manage logout (simply remove correct token and redirect to login form)
 */
void web_server::handleLogout(AsyncWebServerRequest *request)
{
	log_i("Disconnection");
	AsyncWebServerResponse *response = request->beginResponse(301); // Sends 301 redirect
	response->addHeader(F("Location"), F("/login.html?msg=User disconnected"));
	response->addHeader(FPSTR(CacheControlHeader), F("no-cache"));
	response->addHeader(F("Set-Cookie"), F("ESPSESSIONID=0"));
	request->send(response);
	return;
}

void web_server::webLoginUpdate(AsyncWebServerRequest *request)
{
	const auto webUserName = F("webUserName");
	const auto webPassword = F("webPassword");

	log_i("web login Update");

	if (!manageSecurity(request))
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
		handleError(request, F("Correct Parameters not provided"), 400);
	}

	config::instance.save();
	redirectToRoot(request);
}

void web_server::otherSettingsUpdate(AsyncWebServerRequest *request)
{
	const auto hostName = F("hostName");
	const auto ntpServer1 = F("ntpServer1");
	const auto ntpServer2 = F("ntpServer2");
	const auto ntpServerRefreshInterval = F("ntpServerRefreshInterval");
	const auto timezone = F("timezone");

	log_i("config Update");

	if (!manageSecurity(request))
	{
		return;
	}

	if (request->hasArg(hostName))
	{
		config::instance.data.set_host_name(request->arg(hostName));
	}

	config::instance.save();
	redirectToRoot(request);
}

void web_server::restartDevice(AsyncWebServerRequest *request)
{
	log_i("restart");

	if (!manageSecurity(request))
	{
		return;
	}

	request->send(200);
	operations::instance.reboot();
}

void web_server::factoryReset(AsyncWebServerRequest *request)
{
	log_i("factoryReset");

	if (!manageSecurity(request))
	{
		return;
	}

	request->send(200);
	operations::instance.factoryReset();
}

void web_server::rebootOnUploadComplete(AsyncWebServerRequest *request)
{
	log_i("reboot");

	if (!manageSecurity(request))
	{
		return;
	}

	request->send(200);
	operations::instance.reboot();
}

void web_server::handleFileRead(AsyncWebServerRequest *request)
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

	if (!worksWithoutAuth && !isAuthenticated(request))
	{
		log_d("Redirecting to login page");
		path = String(FPSTR(LoginUrl));
	}

	for (size_t i = 0; i < sizeof(staticFilesMap) / sizeof(staticFilesMap[0]); i++)
	{
		const auto entryPath = FPSTR(pgm_read_ptr(&staticFilesMap[i].Path));
		if (path.equalsIgnoreCase(entryPath))
		{
			const auto mediaType = FPSTR(pgm_read_ptr(&staticFilesMap[i].MediaType));
			const auto data = reinterpret_cast<const uint8_t *>(pgm_read_ptr(&staticFilesMap[i].Data));
			const auto size = pgm_read_dword(&staticFilesMap[i].Size);
			const auto zipped = pgm_read_byte(&staticFilesMap[i].Zipped);
			auto response = request->beginResponse_P(200, String(mediaType), data, size);
			if (worksWithoutAuth)
			{
				response->addHeader(FPSTR(CacheControlHeader), F("public, max-age=31536000"));
			}
			if (zipped)
			{
				response->addHeader(F("Content-Encoding"), F("gzip"));
			}
			request->send(response);
			log_v("Served path:%s mimeType: %s size:%s", path.c_str(), FPSTR(mediaType), size);
			return;
		}
	}

	handleNotFound(request);
}

/** Redirect to captive portal if we got a request for another domain.
 * Return true in that case so the page handler do not try to handle the request again. */
bool web_server::isCaptivePortalRequest(AsyncWebServerRequest *request)
{
	if (!isIp(request->host()))
	{
		log_i("Request redirected to captive portal");
		AsyncWebServerResponse *response = request->beginResponse(302, TextPlainMediaType);
		response->addHeader(F("Location"), String(F("http://")) + toStringIp(request->client()->localIP()));
		request->send(response);
		return true;
	}
	return false;
}

void web_server::handleNotFound(AsyncWebServerRequest *request)
{
	if (isCaptivePortalRequest(request))
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

	handleError(request, message, 404);
}

// is this an IP?
bool web_server::isIp(const String &str)
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

String web_server::toStringIp(const IPAddress &ip)
{
	return ip.toString();
}

void web_server::redirectToRoot(AsyncWebServerRequest *request)
{
	AsyncWebServerResponse *response = request->beginResponse(301); // Sends 301 redirect
	response->addHeader(F("Location"), F("/"));
	request->send(response);
}

void web_server::firmwareUpdateUpload(AsyncWebServerRequest *request,
									  const String &filename,
									  size_t index,
									  uint8_t *data,
									  size_t len,
									  bool final)
{
	log_d("firmwareUpdateUpload");

	if (!manageSecurity(request))
	{
		return;
	}

	String error;
	if (!index)
	{
		String md5;

		if (request->hasHeader(FPSTR(MD5Header)))
		{
			md5 = request->getHeader(FPSTR(MD5Header))->value();
		}

		log_i("Expected MD5:%s", md5.c_str());

		if (md5.length() != 32)
		{
			handleError(request, F("MD5 parameter invalid. Check file exists."), 500);
			return;
		}

		if (operations::instance.startUpdate(request->contentLength(), md5, error))
		{
			// success, let's make sure we end the update if the client hangs up
			request->onDisconnect(handleEarlyUpdateDisconnect);
		}
		else
		{
			handleError(request, error, 500);
			return;
		}
	}

	if (operations::instance.isUpdateInProgress())
	{
		if (!operations::instance.writeUpdate(data, len, error))
		{
			handleError(request, error, 500);
		}

		if (final)
		{
			if (!operations::instance.endUpdate(error))
			{
				handleError(request, error, 500);
			}
		}
	}
}

void web_server::restoreConfigurationUpload(AsyncWebServerRequest *request,
											const String &filename,
											size_t index,
											uint8_t *data,
											size_t len,
											bool final)
{
	log_i("restoreConfigurationUpload");

	if (!manageSecurity(request))
	{
		return;
	}

	String error;
	if (!index)
	{
		web_server::instance.restoreConfigData = std::make_unique<std::vector<uint8_t>>();
	}

	for (size_t i = 0; i < len; i++)
	{
		web_server::instance.restoreConfigData->push_back(data[i]);
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
			handleError(request, F("MD5 parameter invalid. Check file exists."), 500);
			return;
		}

		if (!config::instance.restoreAllConfigAsJson(*web_server::instance.restoreConfigData, md5))
		{
			handleError(request, F("Restore Failed"), 500);
			return;
		}
	}
}

void web_server::handleError(AsyncWebServerRequest *request, const String &message, int code)
{
	if (!message.isEmpty())
	{
		log_i("%s", message.c_str());
	}
	AsyncWebServerResponse *response = request->beginResponse(code, TextPlainMediaType, message);
	response->addHeader(FPSTR(CacheControlHeader), F("no-cache, no-store, must-revalidate"));
	response->addHeader(F("Pragma"), F("no-cache"));
	response->addHeader(F("Expires"), F("-1"));
	request->send(response);
}

void web_server::handleEarlyUpdateDisconnect()
{
	operations::instance.abortUpdate();
}

void web_server::notifySensorChange()
{
	if (events.count())
	{
		// events.send(String(hardware::instance.getLux(), 2).c_str(), "lux", millis());
	}
}
