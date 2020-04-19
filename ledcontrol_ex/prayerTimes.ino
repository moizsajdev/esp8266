void updateCalculatedTiming() {
  double times[sizeof(TimeName)/sizeof(char*)];
  set_calc_method(MWL);
  if (configSettings.prayTimeSettings.hanafi > 0) {
    set_asr_method(Hanafi);
  }
//  set_high_lats_adjust_method(MidNight);
 set_high_lats_adjust_method(AngleBased);
 
  set_fajr_angle(configSettings.prayTimeSettings.fajr_angle);
  set_isha_angle(configSettings.prayTimeSettings.isha_angle);

//  set_dhuhr_minutes(0);
  get_prayer_times(year(), month(), day(),
  configSettings.prayTimeSettings.lat,
  configSettings.prayTimeSettings.lon,
  configSettings.prayTimeSettings.tz, times);
//  get_prayer_times(2016, 4, 3, 12.93, 77.59, 5.5, times);
  
  int hours, minutes;
  get_float_time_parts(times[5], hours, minutes);
  calcTiming.magrib = hours*100+minutes + configSettings.prayTimeSettings.magrib_mins;

  get_float_time_parts(times[0], hours, minutes);
  calcTiming.suhr = hours*100+minutes - configSettings.prayTimeSettings.suhr_mins;

  get_float_time_parts(times[1], hours, minutes);
  calcTiming.sunrise = hours*100+minutes;
}

void getSaumTime(int &suhr, int &magrib)
{
  double times[sizeof(TimeName)/sizeof(char*)];
  double currTime=hour()+minute()/60.0;
  int i;
  
  set_calc_method(ISNA);
  set_asr_method(Shafii);
  set_high_lats_adjust_method(AngleBased);
  set_fajr_angle(15);
  set_isha_angle(15);

 
  get_prayer_times(year(), month(), day(), 12.93, 77.59, 5.5, times);
  int hours, minutes;
  get_float_time_parts(times[5], hours, minutes);
  magrib = hours*100+minutes;
  get_float_time_parts(times[0], hours, minutes);
  suhr = hours*100+minutes;

}
