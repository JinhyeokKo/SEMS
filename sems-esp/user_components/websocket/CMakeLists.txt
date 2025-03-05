idf_component_register(SRCS "web_prov.c" "web_server.c" "web_client.c"
  INCLUDE_DIRS "include"
  EMBED_FILES "html/error.html"
              "html/favicon.ico"
              "html/main.js"
              "html/root.html"
  REQUIRES json sdcard wifi_prov common snopnop2002__sqlite3 sqlite)
