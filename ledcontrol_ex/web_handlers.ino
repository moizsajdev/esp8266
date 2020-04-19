
void saveHandler() {
  //Serial.println("saveHandler");
//  logDetails();
//  updateConfigFromTiming();
  EEPROM_write(0, configSettings);
  
   updateCalculatedTiming();
  webServer.send(200, "application/text", "Saved");

}

void notFoundHandler() {
  //Serial.println("Not Found Handler");
  //logDetails();
}


/*
{
    "wifi" : { 
                "apName" : "mymasjid",
                "apsk" : "123"
    },
    "praytimesetting" : {
                "lat" : 12.93,
                "lon" : 77.59,
                "tz" : 5.5,
                "fajr_angle" : 18,
                "isha_angle" : 18,
                "hanafi" : 1,
                "suhr_mins" : 5,
                "magrib_mins" : 5,
                "next_wait_mins" : 5
                "led_intensity" : 6
                
    }
}
*/
void settingPostHandler() {
//  Serial.println("settingPostHandler");
//  logDetails();
   String data = webServer.arg(0);
//  Serial.println("settings data" + data);
 //TODO: Read into temp variable
  if (!settingsFromJSON(data))
  {
//    Serial.println("parseObject() failed");
    webServer.send(401, "application/text", "Parse Failure");
    return;
    
  }

//  Serial.println("parseObject() Success");
  webServer.send(200, "application/text", "Success");
  
  //Setting functions
  set_led_intensity(configSettings.prayTimeSettings.led_intensity);
}

void settingGetHandler() {
//  Serial.println("settingGetHandler");
//  logDetails();

  char jsonString[548];
  settingsToJSON(jsonString, sizeof(jsonString));
  
  webServer.send(200, "application/json", jsonString);


}



String methodToString(HTTPMethod method) {
  switch(method) {
  case HTTP_GET:
    return "GET";
  case HTTP_POST:
    return "POST";
  case HTTP_PUT:
    return "PUT";
  case HTTP_PATCH:
    return "PATCH";
  case HTTP_DELETE:
    return "DELETE";
  }
  return "Unknown";
}
