int EEPROM_write(int ee, struct Config& value)
{
    EEPROM.begin(512);
    const byte* p = (const byte*)(const void*)&value;
    unsigned int i;
    for (i = 0; i < sizeof(value); i++)
      EEPROM.write(ee++, *p++);
    EEPROM.end();
    return i;
}

int EEPROM_read(int ee, struct Config& value)
{
    EEPROM.begin(512);
    byte* p = (byte*)(void*)&value;
    unsigned int i;
    for (i = 0; i < sizeof(value); i++)
      *p++ = EEPROM.read(ee++);
    EEPROM.end();
    return i;
}

bool settingsFromJSON( String json)
{
  const size_t BUFFER_SIZE = JSON_OBJECT_SIZE(12) +
      JSON_OBJECT_SIZE(2)     // the root object has 4 elements
      + JSON_OBJECT_SIZE(2)   // the "wifi" object has 2 elements
      + JSON_OBJECT_SIZE(14);   // the "praytime" object has 2 elements
      
    StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(json);
//    Config newConfig;
    if (root.success()) {
      copyStringHelper(root["wifi"]["apName"],configSettings.wifiSettings.apName);
      copyStringHelper(root["wifi"]["apsk"],configSettings.wifiSettings.wifiAPSK);

      configSettings.prayTimeSettings.lat = root["praytimesetting"]["lat"];
      configSettings.prayTimeSettings.lon = root["praytimesetting"]["lon"];
      configSettings.prayTimeSettings.tz = root["praytimesetting"]["tz"];
      configSettings.prayTimeSettings.fajr_angle = root["praytimesetting"]["fajr_angle"];
      configSettings.prayTimeSettings.isha_angle = root["praytimesetting"]["isha_angle"];
      configSettings.prayTimeSettings.hanafi = root["praytimesetting"]["hanafi"];
      configSettings.prayTimeSettings.suhr_mins = root["praytimesetting"]["suhr_mins"];
      configSettings.prayTimeSettings.magrib_mins = root["praytimesetting"]["magrib_mins"];
      configSettings.prayTimeSettings.sync_interval = root["praytimesetting"]["sync_interval"];
      configSettings.prayTimeSettings.led_intensity = root["praytimesetting"]["led_intensity"];
      configSettings.prayTimeSettings.jumah_enable = root["praytimesetting"]["jumah_enable"];
    }
    
    return root.success();
}

void settingsToJSON(char* json, size_t maxSize) {
 const size_t BUFFER_SIZE = JSON_OBJECT_SIZE(12) +
      JSON_OBJECT_SIZE(2)     // the root object has 4 elements
      + JSON_OBJECT_SIZE(2)   // the "wifi" object has 2 elements
      + JSON_OBJECT_SIZE(14);   // the "praytime" object has 2 elements

    StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    JsonObject& wifi = root.createNestedObject("wifi");
    JsonObject& praytimeSetting = root.createNestedObject("praytimesetting");
    wifi["apName"] = configSettings.wifiSettings.apName;
    wifi["apsk"] = configSettings.wifiSettings.wifiAPSK;

    praytimeSetting["lat"] =  configSettings.prayTimeSettings.lat;
    praytimeSetting["lon"] =  configSettings.prayTimeSettings.lon;
    praytimeSetting["tz"] =  configSettings.prayTimeSettings.tz;
    praytimeSetting["fajr_angle"] =  configSettings.prayTimeSettings.fajr_angle;
    praytimeSetting["isha_angle"] =  configSettings.prayTimeSettings.isha_angle;
    praytimeSetting["hanafi"] =  configSettings.prayTimeSettings.hanafi;
    praytimeSetting["suhr_mins"] =  configSettings.prayTimeSettings.suhr_mins;
    praytimeSetting["magrib_mins"] =  configSettings.prayTimeSettings.magrib_mins;
    praytimeSetting["sync_interval"] =  configSettings.prayTimeSettings.sync_interval;
    praytimeSetting["led_intensity"]  = configSettings.prayTimeSettings.led_intensity;
    praytimeSetting["jumah_enable"]  = configSettings.prayTimeSettings.jumah_enable;

    root.printTo(json, maxSize);
 
}
void copyStringHelper(const char * input, char arr[]) {

  if (strlen(input) < WIFI_STRING_SIZE) {
      strcpy(arr , input);
  } else {
    strncpy(arr,input,WIFI_STRING_SIZE);
    arr[WIFI_STRING_SIZE-1] = '\0';
  }
}
