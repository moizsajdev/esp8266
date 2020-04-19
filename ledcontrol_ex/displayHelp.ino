
/*
  This method will scroll all the hexa-decimal
 numbers and letters on the display. You will need at least
 four 7-Segment digits. otherwise it won't really look that good.
 */
void scrollDigits() {
  for(int i=0;i<13;i++) {
    lc.setDigit(0,7,i,true);
    lc.setDigit(0,6,i+1,false);
    lc.setDigit(0,5,i+2,false);
    lc.setDigit(0,4,i+3,false);
    lc.setDigit(0,3,i+4,false);
    lc.setDigit(0,2,i+5,false);
    lc.setDigit(0,1,i+6,false);
    lc.setDigit(0,0,i+7,false);
    delay(delaytime);
  }
  lc.clearDisplay(0);
  delay(delaytime);
}

void showYear(int year) {
  show4Digit(year, true, false);
}

void show4Digit(int number, bool lsb, bool divider) {
  int pos = lsb ? 0 : 4;
  lc.setDigit(0, pos+0, number%10, false);
  lc.setDigit(0, pos+1, (number/10)%10, false);
  lc.setDigit(0, pos+2, (number/100)%10, divider);
  lc.setDigit(0, pos+3, (number/1000)%10, false);  
}
void showDateMonth(int date, int month) {
  lc.setDigit(0, 0, month%10, false);
  lc.setDigit(0, 1, (month/10)%10, false);
  lc.setDigit(0, 2, date%10, true);
  lc.setDigit(0, 3, (date/10)%10, false);
  
}
void showTime(int hour, int minutes, bool blink) {
  lc.setDigit(0, 4, minutes%10, false);
  lc.setDigit(0, 5, (minutes/10)%10, false);
  lc.setDigit(0, 6, hour%10, blink);
  lc.setDigit(0, 7, (hour/10)%10, false);
  
}

void showSaumTime(int suhr, int magrib) {

  show4Digit(suhr, false, true);
  show4Digit(magrib, true, true);

}
