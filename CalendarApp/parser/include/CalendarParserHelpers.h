/**********
Name: Young Choang Pascale
Student ID: 0985717
Assignment 1
***********************/

#ifndef CALENDARPARSERHELPERS_H
#define CALENDARPARSERHELPERS_H

#include "CalendarParser.h"

Event* initializeEvent();
ICalErrorCode parseEvent(FILE* fp, Event* event);
Property * initializeProperty(char * propDescrip, char * propName);
Alarm * initializeAlarm();
ICalErrorCode parseAlarm(FILE *fp, Alarm *alarm);
ICalErrorCode returnErrorCode(Calendar* tempObj, Calendar** obj, FILE* fp, ICalErrorCode errorCode);
ICalErrorCode validateDateTime(char string[]);
bool checkLineEnding(char string[]) ;

ICalErrorCode writeEvent(FILE* fp, const Event* event);
ICalErrorCode writeDateTime(FILE* fp, const DateTime tempDT, int type);
ICalErrorCode writeAlarm(FILE* fp, const Alarm* tempAlarm);
ICalErrorCode writeProperty(FILE* FP, const Property* tempProp);
ICalErrorCode validateEvent(Event* event);
ICalErrorCode validateAlarm(Alarm* alarm);

char* getCalJSON(char* filename);
char* getEventListJSON(char* filename);

char *alarmToJSON(const Alarm *alarm);
char *alarmListToJSON(const List *alarmList);
char *PropertyToJSON(const Property* prop);
char *propListToJSON(const List *propList);
char* checkFileValidity(char* filename);
char *getAlarmListJSON(char *filename, int eventNo);
char *getPropListJSON(char *filename, int eventNo);
char* createEvt(char* filename, char* uid, char* stampDate, char* stampTime, char* startTime, char* startDate,char* summary, int stampUTC, int startUTC);
void addEventToCal(Event* event, Calendar* cal, char* stampDate, char* stampTime, char* startTime, char* startDate,char* summary, int stampUTC, int startUTC);
char* createNewCal(char* filename, char* calJSON, char* uid, char* stampDate, char* stampTime, char* startTime, char* startDate,char* summary, int stampUTC, int startUTC);

#endif
