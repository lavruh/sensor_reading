#include <M5Unified.h>

#include "field.h"
#include "menu.h"

extern TitleField titleField;
extern TitleField body1;
extern LogField logField;

namespace {
bool isLeapYear(uint16_t year) {
  return year % 4 == 0 && (year % 100 != 0 || year % 400 == 0);
}

uint8_t daysInMonth(uint16_t year, uint8_t month) {
  constexpr uint8_t monthLengths[] = {31, 28, 31, 30, 31, 30,
                                      31, 31, 30, 31, 30, 31};
  if (month == 2 && isLeapYear(year)) {
    return 29;
  }
  return monthLengths[month - 1];
}
}

void SetTimeMenuStrategy::loopTask() {
  if (editing) {
    showCurrentStep();
    return;
  }

  titleField.show("Set Time");
  if (rtcError != nullptr) {
    logField.show(String(rtcError) + "\nA: retry");
  } else if (cancelled) {
    logField.show("Cancelled\nA: restart");
  } else if (!updated) {
    logField.show("Press A to set");
  } else {
    logField.show("Updated\nA: edit");
  }
}

void SetTimeMenuStrategy::onMenuEnter() {
  titleField.redraw();
  logField.redraw();
}

void SetTimeMenuStrategy::onButtonA() {
  if (!editing) {
    beginEditing();
    return;
  }

  confirmStep();
}

void SetTimeMenuStrategy::onButtonALongPress() {
  if (!editing) {
    return;
  }
  editing = false;
  updated = false;
  cancelled = true;
  showCurrentStep();
}

void SetTimeMenuStrategy::beginEditing() {
  rtcError = nullptr;
  updated = false;
  cancelled = false;
  if (!M5.Rtc.isEnabled()) {
    rtcError = "RTC unavailable";
    return;
  }

  m5::rtc_datetime_t datetime{};
  if (!M5.Rtc.getDateTime(&datetime)) {
    rtcError = "RTC read failed";
    return;
  }

  if (datetime.date.year < 1900 || datetime.date.year > 2099 ||
      datetime.date.month < 1 || datetime.date.month > 12 ||
      datetime.date.date < 1 ||
      datetime.date.date > daysInMonth(datetime.date.year,
                                       datetime.date.month) ||
      datetime.time.hours > 23 || datetime.time.minutes > 59 ||
      datetime.time.seconds > 59) {
    rtcError = "Invalid RTC time";
    return;
  }

  year = datetime.date.year;
  month = datetime.date.month;
  day = datetime.date.date;
  hours = datetime.time.hours;
  minutes = datetime.time.minutes;
  seconds = datetime.time.seconds;
  step = Step::year;
  editing = true;
  showCurrentStep();
}

void SetTimeMenuStrategy::confirmStep() {
  switch (step) {
    case Step::year:
      step = Step::month;
      break;
    case Step::month:
      step = Step::day;
      break;
    case Step::day:
      step = Step::hours;
      break;
    case Step::hours:
      step = Step::minutes;
      break;
    case Step::minutes:
      step = Step::seconds;
      break;
    case Step::seconds: {
      m5::rtc_datetime_t datetime{};
      datetime.date.year = year;
      datetime.date.month = month;
      datetime.date.date = day;
      datetime.time.hours = hours;
      datetime.time.minutes = minutes;
      datetime.time.seconds = seconds;
      M5.Rtc.setDateTime(&datetime);
      M5.Rtc.setSystemTimeFromRtc();
      editing = false;
      updated = true;
      cancelled = false;
      break;
    }
  }
  showCurrentStep();
}

bool SetTimeMenuStrategy::onButtonB() {
  if (!editing) {
    return false;
  }
  adjustValue(1);
  showCurrentStep();
  return true;
}

bool SetTimeMenuStrategy::onButtonPower() {
  if (!editing) {
    return false;
  }
  adjustValue(-1);
  showCurrentStep();
  return true;
}

void SetTimeMenuStrategy::adjustValue(int8_t direction) {
  switch (step) {
    case Step::year:
      if (direction > 0) {
        year = year >= 2099 ? 1900 : year + 1;
      } else {
        year = year <= 1900 ? 2099 : year - 1;
      }
      if (day > daysInMonth(year, month)) {
        day = daysInMonth(year, month);
      }
      break;
    case Step::month:
      if (direction > 0) {
        month = month >= 12 ? 1 : month + 1;
      } else {
        month = month <= 1 ? 12 : month - 1;
      }
      if (day > daysInMonth(year, month)) {
        day = daysInMonth(year, month);
      }
      break;
    case Step::day: {
      const uint8_t lastDay = daysInMonth(year, month);
      if (direction > 0) {
        day = day >= lastDay ? 1 : day + 1;
      } else {
        day = day <= 1 ? lastDay : day - 1;
      }
      break;
    }
    case Step::hours:
      hours = direction > 0 ? (hours + 1) % 24 : (hours + 23) % 24;
      break;
    case Step::minutes:
      minutes = direction > 0 ? (minutes + 1) % 60 : (minutes + 59) % 60;
      break;
    case Step::seconds:
      seconds = direction > 0 ? (seconds + 1) % 60 : (seconds + 59) % 60;
      break;
  }
}

void SetTimeMenuStrategy::showCurrentStep() {
  if (!editing) {
    titleField.show("Set Time");
    if (rtcError != nullptr) {
      logField.show(String(rtcError) + "\nA: retry");
    } else if (cancelled) {
      logField.show("Cancelled\nA: restart");
    } else {
      logField.show(updated ? "Updated\nA: edit" : "Press A to set");
    }
    return;
  }

  const char* label = "";
  uint16_t value = 0;
  switch (step) {
    case Step::year:
      label = "Y";
      value = year;
      break;
    case Step::month:
      label = "M";
      value = month;
      break;
    case Step::day:
      label = "D";
      value = day;
      break;
    case Step::hours:
      label = "Hr";
      value = hours;
      break;
    case Step::minutes:
      label = "Min";
      value = minutes;
      break;
    case Step::seconds:
      label = "Sec";
      value = seconds;
      break;
  }

  titleField.show("Set Time");
  body1.show(String(label) + ": " + String(value));
  logField.show("B/P +/- A=next\nHold A=cancel");
}
