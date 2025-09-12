#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "WebServer.h"
#include "nlohmann/json.hpp"
#include <codecvt>
#include <locale>
#include <chrono>
#include <future>
#include <string>
#include "../logging/Logger.hpp"
#include "../utils/StringUtils.hpp"

namespace {

std::string HtmlEscape_(std::wstring_view s) {
  std::wstring out;
  out.reserve(s.size());
  for (wchar_t c : s)
    switch (c) {
      case L'&':
        out += L"&amp;";
        break;
      case L'<':
        out += L"&lt;";
        break;
      case L'>':
        out += L"&gt;";
        break;
      case L'"':
        out += L"&quot;";
        break;
      case L'\'':
        out += L"&#39;";
        break;
      default:
        out += c;
    }
  return StringUtils::wideStringToString(out);
}

std::string RenderIndex(Settings& settings) {
  std::ostringstream html, buttons;

  for (const std::wstring_view n : settings.getAllConfigurationNames()) {
    const auto label = HtmlEscape_(n);
    const auto value = HtmlEscape_(n);  // safe for attribute too
    buttons << "<button class='cfg' data-name=\"" << value << "\">" << label
            << "</button>\n";
  }

  html << "<!doctype html><html lang='en'><head>"
       << "<meta charset='utf-8'/>"
       << "<meta name='viewport' content='width=device-width,initial-scale=1'/>"
       << "<link rel='icon' "
          "href='https://raw.githubusercontent.com/shamskv/EZVIDIA/master/"
          "EZVIDIA/resources/logo.ico'>"
       << "<link rel='manifest' href='/manifest.webmanifest'>"
       << "<link rel='apple-touch-icon' "
          "href='https://<your-user>.github.io/EZVIDIA/assets/"
          "apple-touch-icon.png'>"
       << "<meta name='theme-color' content='#0ea5a5'>"
       << "<title>EZVIDIA</title>"
       << "<style>"
          " :root{--bg:#111418;--card:#1b1f24;--text:#e7e9ec;--muted:#b3b9c5;"
          "       --accent:#10b981;--accent-2:#059669;--border:#2a2f36;}"
          " *{box-sizing:border-box;-webkit-tap-highlight-color:transparent}"
          " html,body{margin:0;background:var(--bg);color:var(--text);"
          "   font-family:system-ui,-apple-system,'Segoe "
          "UI',Roboto,Arial,sans-serif}"
          " .wrap{max-width:760px;margin:0 auto;padding:20px}"
          " .bar{display:flex;align-items:center;gap:12px;background:#0ea5a5;"
          "      padding:14px 18px;position:sticky;top:0;z-index:1;"
          "      box-shadow:0 2px 10px rgba(0,0,0,.25)}"
          " .title{font-weight:700;letter-spacing:.5px}"
          " .card{background:var(--card);border:1px solid var(--border);"
          "       border-radius:14px;padding:16px;margin-top:16px;"
          "       box-shadow:0 6px 24px rgba(0,0,0,.25)}"
          " .header-row{display:flex;align-items:center;justify-content:space-"
          "between;margin-bottom:20px}"
          " h1{font-size:1rem;margin:0}"
          " .chip{padding:8px 14px;border-radius:999px;border:1px solid "
          "var(--border);"
          "   background:#22272e;color:var(--muted);cursor:pointer;}"
          " .grid{display:grid;grid-template-columns:1fr;gap:12px}"
          " @media(min-width:520px){.grid{grid-template-columns:1fr}}"
          " button.cfg{width:100%;text-align:center;padding:16px 14px;"
          "   font-weight:700;letter-spacing:.6px;text-transform:uppercase;"
          "   border-radius:12px;border:1px solid var(--border);"
          "   background:#2a2f36;color:var(--text);"
          "   box-shadow:0 2px 10px rgba(0,0,0,.15);"
          "   transition:transform .06s ease,box-shadow .06s ease,background "
          ".2s}"
          " button.cfg:active{transform:scale(.98);box-shadow:0 1px 6px "
          "rgba(0,0,0,.25)}"
          " #log{margin-top:14px;font-size:.9rem;color:var(--muted);white-"
          "space:pre-wrap}"
       << "</style></head><body>"
       << "<div class='bar'>"
          "<img "
          "src='https://raw.githubusercontent.com/shamskv/EZVIDIA/master/"
          "EZVIDIA/resources/logo.ico' "
          "     alt='EZVIDIA' width='20' height='20' "
          "style='filter:brightness(0) invert(1)'>"
          "<div class='title'>EZVIDIA</div>"
       << "</div>"
       << "<div class='wrap'>"
          "<div class='card'>"
          "<div class='header-row'>"
          "<h1>My configurations</h1>"
          "<button class='chip' onclick='refreshPage()'>Refresh</button>"
          "</div>"
          "<div class='grid' id='configs'>"
       << buttons.str()
       << "</div>"
          "<div id='log'></div>"
          "</div>"
       << "</div>"
       << "<script>"
          "const cfgBox=document.getElementById('configs');"
          "cfgBox.addEventListener('click',e=>{"
          "const btn=e.target.closest('button.cfg');"
          "if(!btn) return; apply(btn.dataset.name);"
          "});"
          "function refreshPage(){ location.reload(); }"
          "let busy=false;"
          "async function apply(name){"
          "if(busy) return; busy=true;"
          "try{"
          "const r=await "
          "fetch('/apply',{method:'POST',headers:{'Content-Type':'application/"
          "json'},body:JSON.stringify({name})});"
          "const j=await r.json().catch(()=>({}));"
          "log(new Date().toLocaleTimeString()+`  ${name} -> ${r.status} "
          "${JSON.stringify(j)}`);"
          "}catch(e){log(`apply(${name}) -> error: ${e}`);}finally{busy=false;}"
          "}"
          "function log(s){const "
          "el=document.getElementById('log');el.textContent+=s+'\\n';el."
          "scrollTop=el.scrollHeight;}"
       << "</script></body></html>";

  return html.str();
}
}  // namespace

WebServer::WebServer(Settings* settings, DisplayDriver* driver)
    : settings_(*settings), driver_(*driver) {
  server_ = std::make_unique<httplib::Server>();
  server_->set_default_headers({{"Cache-Control", "no-store"}});

  // GET "/": one simple page with a button per config
  server_->Get("/", [this](const httplib::Request&, httplib::Response& res) {
    res.set_content(RenderIndex(settings_), "text/html; charset=utf-8");
  });

  // POST "/apply": { "name": "…" }
  server_->Post("/apply", [this](const httplib::Request& req,
                                 httplib::Response& res) {
    std::string name;
    if (!req.body.empty()) {
      try {
        auto j = nlohmann::json::parse(req.body);
        name = j.value("name", "");
      } catch (...) {
      }
    }
    if (name.empty()) {
      res.status = 400;
      res.set_content(R"({"ok":false,"error":"missing name"})",
                      "application/json");
      return;
    }
    std::wstring wname = StringUtils::stringToWideString(name);
    std::optional<GlobalConfiguration> conf = settings_.getConfiguration(wname);
    bool ok = false;
    if (conf.has_value()) {
      ok = driver_.applyConfig(*conf);
    }
    nlohmann::json out = {{"ok", ok}, {"name", name}};
    res.set_content(out.dump(), "application/json");
  });

  // manifest for the nice icon when "installing" the web app.
  server_->Get("/manifest.webmanifest", [](const httplib::Request&,
                                           httplib::Response& res) {
    const std::string base = "https://shamskv.github.io/EZVIDIA/assets/";

    nlohmann::json manifest = {
        {"name", "EZVIDIA"},
        {"short_name", "EZVIDIA"},
        {"start_url", "/"},
        {"display", "standalone"},
        {"background_color", "#111418"},
        {"theme_color", "#0ea5a5"},
        {"icons",
         nlohmann::json::array(
             {nlohmann::json{{"src", base + "android-chrome-192x192.png"},
                             {"sizes", "192x192"},
                             {"type", "image/png"}},
              nlohmann::json{{"src", base + "android-chrome-512x512.png"},
                             {"sizes", "512x512"},
                             {"type", "image/png"}}})}};

    res.set_content(manifest.dump(), "application/manifest+json");
  });

  // Start server thread
  th_ = std::thread([this] { server_->listen("0.0.0.0", 3737); });
  for (int i = 0; i < 100; i++) {  // ~1s timeout
    if (server_->is_running()) {
      return;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  // Timed out waiting for server to start.
  error_ = true;
}

WebServer::~WebServer() {
  if (server_)
    server_->stop();
  if (th_.joinable())
    th_.join();
  server_.reset();
}