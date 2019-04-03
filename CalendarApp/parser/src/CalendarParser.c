/**********
Name: Young Choang Pascale
Student ID: 0985717
Assignment 2
***********************/

#include "CalendarParser.h"
#include "LinkedListAPI.h"
#include "CalendarParserHelpers.h"

/** Function to create a Calendar object based on the contents of an iCalendar file.
 *@pre File name cannot be an empty string or NULL.  File name must have the .ics extension.
       File represented by this name must exist and must be readable.
 *@post Either:
        A valid calendar has been created, its address was stored in the variable obj, and OK was returned
		or
		An error occurred, the calendar was not created, all temporary memory was freed, obj was set to NULL, and the
		appropriate error code was returned
 *@return the error code indicating success or the error encountered when parsing the calendar
 *@param fileName - a string containing the name of the iCalendar file
 *@param a double pointer to a Calendar struct that needs to be allocated
**/
ICalErrorCode createCalendar(char *fileName, Calendar **obj)
{
	ICalErrorCode errorCode;
	FILE *openFile;
	char *token;
	char *tokenPropDescrip;
	char *tempFile;
	char line[3000];
	char tempLine[3000];
	char comment[3000];
	int firstChar;		  //Use to check if beginning of line is space or tab
	int versionCount = 0; //maybe do not need these. just check if stuffs in the calendar struch has been changed
	int prodidCount = 0;
	int calComponent = 0;
	int beginCalCount = 0;
	bool beginCal = false;
	bool endCal = false;

	errorCode = OK;

	//Initializie the Calendar object
	Calendar *tempObj = malloc(sizeof(Calendar));
	tempObj->version = 0.0;
	tempObj->prodID[0] = '\0';
	tempObj->events = initializeList(&printEvent, &deleteEvent, &compareEvents);
	tempObj->properties = initializeList(&printProperty, &deleteProperty, &compareProperties);

	if (fileName == NULL || strcmp(fileName, "") == 0)
	{
		return returnErrorCode(tempObj, obj, NULL, INV_FILE);
	}

	if (!obj)
	{
		return returnErrorCode(tempObj, obj, NULL, OTHER_ERROR);
	}

	//checks if filename ends with .ics
	//use of a tempFile cause using strtok it will alter the fileName
	tempFile = malloc(sizeof(char) * (strlen(fileName) + 1));
	strcpy(tempFile, fileName);
	token = strtok(tempFile, ".");
	if (token != NULL)
	{
		token = strtok(NULL, ".");
	}
	if (token == NULL || strcmp(token, "ics") != 0)
	{
		free(tempFile);
		return returnErrorCode(tempObj, obj, NULL, INV_FILE);
	}
	free(tempFile);

	//Reading the file here:
	openFile = fopen(fileName, "r");
	if (openFile == NULL)
	{
		return returnErrorCode(tempObj, obj, NULL, INV_FILE);
	}

	while (fgets(line, sizeof(line), openFile) != NULL)
	{
		if (!checkLineEnding(line))
		{
			return returnErrorCode(tempObj, obj, openFile, INV_FILE);
		}

		if (endCal)
		{
			//return error because after VCALENDAR there shouldnt be any more lines to read
			return returnErrorCode(tempObj, obj, openFile, INV_CAL);
		}

		//checking for folding lines
		while ((firstChar = fgetc(openFile)) == ' ' || firstChar == '\t' || firstChar == ';')
		{
			if (firstChar == ';')
			{
				fgets(comment, sizeof(comment), openFile);
			}
			else
			{
				fgets(tempLine, sizeof(tempLine), openFile);
				line[strlen(line) - 2] = '\0';
				strcat(line, tempLine);
				tempLine[0] = '\0';
			}
		}

		if (fgetc(openFile) != EOF)
		{
			fseek(openFile, -2, SEEK_CUR);
		}

		token = strtok(line, ";:");
		int i = 0;
		while (token[i] != '\0')
		{
			if (token[i] >= 'a' && token[i] <= 'z')
			{
				token[i] = token[i] - 32;
			}
			i++;
		}
		if (strcmp(token, "VERSION") == 0)
		{
			token = strtok(NULL, "\r\n");
			if (token == NULL)
			{
				return returnErrorCode(tempObj, obj, openFile, INV_VER);
			}
			float tempVersion = atof(token);
			if (tempVersion == 0.0)
			{
				return returnErrorCode(tempObj, obj, openFile, INV_VER);
			}
			tempObj->version = tempVersion;
			versionCount++;
			if (versionCount > 1)
			{
				return returnErrorCode(tempObj, obj, openFile, DUP_VER);
			}
		}
		else if (strcmp(token, "PRODID") == 0)
		{
			token = strtok(NULL, "\r\n");
			if (token == NULL)
			{
				return returnErrorCode(tempObj, obj, openFile, INV_PRODID);
			}
			strcpy(tempObj->prodID, token);
			prodidCount++;
			if (prodidCount > 1)
			{
				return returnErrorCode(tempObj, obj, openFile, DUP_PRODID);
			}
		}
		else if (strcmp(token, "BEGIN") == 0)
		{
			token = strtok(NULL, "\r\n");

			if (token == NULL)
			{
				return returnErrorCode(tempObj, obj, openFile, INV_CAL);
			}

			if (strcmp(token, "VEVENT") == 0)
			{
				Event *tempEvent = initializeEvent();
				if (tempEvent == NULL)
				{
					return returnErrorCode(tempObj, obj, openFile, OTHER_ERROR);
				}
				//parsing tempEvent cause tempEvent's value contains the address where memory is allocated
				errorCode = parseEvent(openFile, tempEvent);
				if (errorCode == OK)
				{
					insertBack(tempObj->events, (void *)tempEvent);
					calComponent++;
					//printf("event uid: %s, dtdate: %s and sttime: %s\n", tempEvent->UID, tempEvent->creationDateTime.date, tempEvent->creationDateTime.time);
				}
				else
				{
					deleteEvent((void *)tempEvent);
					return returnErrorCode(tempObj, obj, openFile, errorCode);
				}
			}
			else if (strcmp(token, "VCALENDAR") == 0)
			{
				if (beginCalCount > 1)
				{
					return returnErrorCode(tempObj, obj, openFile, INV_CAL);
				}
				beginCal = true;
				beginCalCount++;
			}
			else
			{
				//Second token missing i.e BEGIN:
				return returnErrorCode(tempObj, obj, openFile, INV_CAL);
			}
		}
		else if (strcmp(token, "END") == 0)
		{
			token = strtok(NULL, "\r\n");
			if (token == NULL)
			{
				return returnErrorCode(tempObj, obj, openFile, INV_CAL);
			}
			if (strcmp(token, "VCALENDAR") == 0)
			{
				endCal = true;
			}
		}
		else
		{ //other calendar properties
			tokenPropDescrip = strtok(NULL, "\r\n");
			if (tokenPropDescrip == NULL)
			{
				return returnErrorCode(tempObj, obj, openFile, INV_CAL);
			}
			Property *tempProperty = initializeProperty(tokenPropDescrip, token);
			if (tempProperty == NULL)
			{
				return returnErrorCode(tempObj, obj, openFile, OTHER_ERROR);
			}
			insertBack(tempObj->properties, (void *)tempProperty);
		}
		if (!beginCal)
		{
			beginCalCount = -1;
			return returnErrorCode(tempObj, obj, openFile, INV_CAL);
		}
	}

	// Checking that version, prodid and a cal component are present in the calendar
	if (versionCount == 0 || calComponent == 0 || prodidCount == 0 || endCal == false)
	{
		return returnErrorCode(tempObj, obj, openFile, INV_CAL);
	}

	fclose(openFile);
	*obj = tempObj;

	return errorCode;
}

ICalErrorCode returnErrorCode(Calendar *tempObj, Calendar **obj, FILE *fp, ICalErrorCode errorCode)
{
	obj = NULL;
	deleteCalendar(tempObj);
	if (fp != NULL)
	{
		fclose(fp);
	}
	return errorCode;
}

bool checkLineEnding(char string[])
{
	if (strlen(string) != 0)
	{
		if (string[strlen(string) - 1] == '\n' && string[strlen(string) - 2] == '\r')
		{
			//if (strlen(string) > 75) {
			//	return false;
			//} else {
			return true;
			//}
		}
	}
	return false;
}

Event *initializeEvent()
{
	Event *event;
	event = calloc(sizeof(Event), 1);
	if (event != NULL)
	{
		event->UID[0] = '\0';
		event->creationDateTime.date[0] = '\0';
		event->creationDateTime.time[0] = '\0';
		event->creationDateTime.UTC = true;
		event->startDateTime.date[0] = '\0';
		event->startDateTime.time[0] = '\0';
		event->startDateTime.UTC = true;
		event->properties = initializeList(&printProperty, &deleteProperty, &compareProperties);
		event->alarms = initializeList(&printAlarm, &deleteAlarm, &compareAlarms);
		return event;
	}
	else
	{
		return NULL;
	}
}

Property *initializeProperty(char *propDescrip, char *propName)
{
	if (strlen(propDescrip) == 0 || strlen(propName) == 0)
	{
		return NULL;
	}
	//Property * property = calloc(sizeof(Property) + (sizeof(char) * strlen(propDescrip) + 1),1);
	Property *property = (Property *)malloc(sizeof(Property) + (sizeof(char) * strlen(propDescrip) + 1));
	if (property != NULL)
	{
		strcpy(property->propName, propName);
		strcpy(property->propDescr, propDescrip);
		return property;
	}
	else
	{
		return NULL;
	}
}

Alarm *initializeAlarm()
{
	Alarm *alarm = calloc(sizeof(Alarm), 1);
	if (alarm != NULL)
	{
		alarm->action[0] = '\0';
		alarm->trigger = malloc(sizeof(char));
		strcpy(alarm->trigger, "");
		alarm->properties = initializeList(&printProperty, &deleteProperty, &compareProperties);
		return alarm;
	}
	else
	{
		return NULL;
	}
}

ICalErrorCode validateDateTime(char string[])
{
	ICalErrorCode errorCode;
	char *token;

	if (strlen(string) == 16 || strlen(string) == 15)
	{
		token = strtok(string, "T");
		if (token == NULL)
		{
			errorCode = INV_DT;
			return errorCode;
		}
		if (strlen(token) != 8)
		{
			errorCode = INV_DT;
			return errorCode;
		}
		token = strtok(NULL, " \0");
		if (token == NULL)
		{

			errorCode = INV_DT;
			return errorCode;
		}
		if (strlen(token) == 6 || strlen(token) == 7)
		{
			if (token[strlen(token) - 1] != 'Z' && strlen(token) == 7)
			{
				errorCode = INV_DT;
				return errorCode;
			}
		}
		else
		{
			errorCode = INV_DT;
			return errorCode;
		}
	}
	else
	{
		errorCode = INV_DT;
		return errorCode;
	}

	errorCode = OK;
	return errorCode;
}

ICalErrorCode parseEvent(FILE *fp, Event *event)
{
	ICalErrorCode errorCode;
	ICalErrorCode dtErrorCode;
	char *token;
	char *tokenPropDescrip;
	char *tempToken;
	char firstChar;
	char line[3000];
	char tempLine[3000];
	char comment[3000];
	int uidPresent = 0;
	int dtstampPresent = 0;
	int dtstartPresent = 0;
	int dtendCount = 0;
	int durationCount = 0;

	while (fgets(line, sizeof(line), fp))
	{
		if (!checkLineEnding(line))
		{
			errorCode = INV_FILE;
			return errorCode;
		}

		while ((firstChar = fgetc(fp)) == ' ' || firstChar == '\t' || firstChar == ';')
		{
			if (firstChar == ';')
			{
				fgets(comment, sizeof(comment), fp);
			}
			else
			{
				fgets(tempLine, sizeof(tempLine), fp);
				line[strlen(line) - 2] = '\0';
				strcat(line, tempLine);
				tempLine[0] = '\0';
			}
		}

		if (fgetc(fp) != EOF)
		{
			fseek(fp, -2, SEEK_CUR);
		}

		token = strtok(line, ";:");
		int i = 0;
		while (token[i] != '\0')
		{
			if (token[i] >= 'a' && token[i] <= 'z')
			{
				token[i] = token[i] - 32;
			}
			i++;
		}
		if (strcmp(token, "UID") == 0)
		{
			token = strtok(NULL, "\r\n");
			if (token == NULL)
			{
				errorCode = INV_EVENT;
				return errorCode;
			}
			strcpy(event->UID, token);
			uidPresent++;
			if (uidPresent > 1)
			{
				errorCode = INV_EVENT;
				return errorCode;
			}
		}
		else if (strcmp(token, "DTSTAMP") == 0)
		{
			token = strtok(NULL, "\r\n");

			if (token == NULL)
			{
				errorCode = INV_EVENT;
				return errorCode;
			}
			tempToken = malloc(sizeof(char) * (strlen(token) + 1));
			strcpy(tempToken, token);
			dtErrorCode = validateDateTime(tempToken);
			free(tempToken);

			if (dtErrorCode == OK)
			{
				strncpy(event->creationDateTime.date, token, 8);
				if (token[strlen(token) - 1] != 'Z')
				{
					event->creationDateTime.UTC = false;
					strncpy(event->creationDateTime.time, token + 9, 15 - 9);
				}
				else
				{
					strncpy(event->creationDateTime.time, token + 9, 15 - 9);
				}

				dtstampPresent++;
				if (dtstampPresent > 1)
				{
					errorCode = INV_EVENT;
					return errorCode;
				}
			}
			else
			{
				errorCode = INV_DT;
				return errorCode;
			}
		}
		else if (strncmp(token, "DTSTART", 7) == 0)
		{
			token = strtok(NULL, "\r\n");

			if (token == NULL)
			{
				errorCode = INV_EVENT;
				return errorCode;
			}

			tempToken = malloc(sizeof(char) * (strlen(token) + 1));
			strcpy(tempToken, token);
			dtErrorCode = validateDateTime(tempToken);
			free(tempToken);

			if (dtErrorCode == OK)
			{
				strncpy(event->startDateTime.date, token, 8);
				if (token[strlen(token) - 1] != 'Z')
				{
					event->startDateTime.UTC = false;
					strncpy(event->startDateTime.time, token + 9, 15 - 9);
				}
				else
				{
					strncpy(event->startDateTime.time, token + 9, 15 - 9);
				}
				//event->startDateTime = stDateTime;
				dtstartPresent++;
				if (dtstartPresent > 1)
				{
					errorCode = INV_EVENT;
					return errorCode;
				}
			}
			else
			{
				errorCode = INV_DT;
				return errorCode;
			}
		}
		else if (strcmp(token, "BEGIN") == 0)
		{
			token = strtok(NULL, "\r\n");
			if (token == NULL)
			{
				errorCode = INV_EVENT;
				return errorCode;
			}
			if (strcmp(token, "VALARM") == 0)
			{
				Alarm *tempAlarm = initializeAlarm();
				if (tempAlarm == NULL)
				{
					errorCode = OTHER_ERROR;
					return errorCode;
				}
				errorCode = parseAlarm(fp, tempAlarm);
				if (errorCode == OK)
				{
					insertBack(event->alarms, (void *)tempAlarm);
				}
				else
				{
					deleteAlarm((void *)tempAlarm);
					errorCode = INV_ALARM;
					return errorCode;
				}
			}
			else
			{
				errorCode = INV_EVENT;
				return errorCode;
			}
			//end of event calendar
		}
		else if (strcmp(token, "END") == 0)
		{
			//printf("print end:vevent\n");
			token = strtok(NULL, "\r\n");
			if (strcmp(token, "VEVENT") == 0)
			{ //valid event component, check if event is a valid one
				//printf("event ends here\n");
				if (uidPresent == 0 || dtstampPresent == 0 || dtstartPresent == 0)
				{
					errorCode = INV_EVENT;
					return errorCode;
				}
				if (dtendCount == 1 && durationCount == 1)
				{
					errorCode = INV_EVENT;
					return errorCode;
				}
				errorCode = OK;
				return errorCode;
			}
			else
			{
				errorCode = INV_EVENT;
				return errorCode;
			}
			//Properties
		}
		else
		{
			//printf("Event properties\n");
			tokenPropDescrip = strtok(NULL, "\r\n");
			if (tokenPropDescrip == NULL)
			{
				errorCode = INV_EVENT;
				return errorCode;
			}
			Property *tempProperty = initializeProperty(tokenPropDescrip, token);
			if (strcmp(token, "CLASS") == 0 || strcmp(token, "CREATED") == 0 || strcmp(token, "DESCRIPTION") == 0 || strcmp(token, "GEO") == 0 || strcmp(token, "LAST-MOD") == 0 || strcmp(token, "LOCATION") == 0 || strcmp(token, "ORGANIZER") == 0 || strcmp(token, "PRIORITY") == 0 || strcmp(token, "SEQ") == 0 || strcmp(token, "STATUS") == 0 || strcmp(token, "SUMMARY") == 0 || strcmp(token, "TRANSP") == 0 || strcmp(token, "URL") == 0 || strcmp(token, "RECURID") == 0 || strcmp(token, "RRULE") == 0)
			{
				ListIterator iter = createIterator(event->properties);
				int match = 0;
				void *element;
				while ((element = nextElement(&iter)) != NULL)
				{
					match = compareProperties(element, (void *)tempProperty);
					if (match == 0)
					{
						free(tempProperty);
						errorCode = INV_EVENT;
						return errorCode;
					}
				}
			}
			if (strcmp(token, "DTEND") == 0)
			{
				dtendCount++;
			}
			if (strcmp(token, "DURATION") == 0)
			{
				durationCount++;
			}
			insertBack(event->properties, (void *)tempProperty);
		}
	}

	errorCode = INV_EVENT;
	return errorCode;
}

ICalErrorCode parseAlarm(FILE *fp, Alarm *alarm)
{
	ICalErrorCode errorCode;
	char *token;
	char *tokenPropDescrip;
	char firstChar;
	char line[3000];
	char tempLine[3000];
	char comment[3000];
	int actionPresent = 0;
	int triggerPresent = 0;
	Property *tempProperty;
	int len = 0;

	errorCode = OK;

	while (fgets(line, sizeof(line), fp))
	{
		if (!checkLineEnding(line))
		{
			errorCode = INV_FILE;
			return errorCode;
		}

		while ((firstChar = fgetc(fp)) == ' ' || firstChar == '\t' || firstChar == ';')
		{
			if (firstChar == ';')
			{
				fgets(comment, sizeof(comment), fp);
			}
			else
			{
				fgets(tempLine, sizeof(tempLine), fp);
				line[strlen(line) - 2] = '\0';
				strcat(line, tempLine);
				tempLine[0] = '\0';
			}
		}

		if (fgetc(fp) != EOF)
		{
			fseek(fp, -2, SEEK_CUR);
		}

		token = strtok(line, ";:");
		int i = 0;
		while (token[i] != '\0')
		{
			if (token[i] >= 'a' && token[i] <= 'z')
			{
				token[i] = token[i] - 32;
			}
			i++;
		}

		if (strcmp(token, "ACTION") == 0)
		{
			token = strtok(NULL, "\r\n");
			if (token == NULL)
			{
				errorCode = INV_ALARM;
				return errorCode;
			}
			strcpy(alarm->action, token);
			actionPresent++;
			if (actionPresent > 1)
			{
				errorCode = INV_ALARM;
				return errorCode;
			}
		}
		else if (strcmp(token, "TRIGGER") == 0)
		{
			token = strtok(NULL, "\r\n");
			if (token == NULL)
			{
				errorCode = INV_ALARM;
				return errorCode;
			}
			len = strlen(token);
			alarm->trigger = realloc(alarm->trigger, (len + 1) * sizeof(char));
			strcpy(alarm->trigger, token);
			triggerPresent++;
			if (triggerPresent > 1)
			{
				errorCode = INV_ALARM;
				return errorCode;
			}
		}
		else if (strcmp(token, "END") == 0)
		{
			token = strtok(NULL, "\r\n");
			if (strcmp(token, "VALARM") == 0)
			{
				if (actionPresent == 0 || triggerPresent == 0)
				{
					errorCode = INV_ALARM;
					return errorCode;
				}
				errorCode = OK;
				return errorCode;
			}
			else
			{
				errorCode = INV_ALARM;
				return errorCode;
			}
		}
		else
		{
			tokenPropDescrip = strtok(NULL, "\r\n");
			if (tokenPropDescrip == NULL)
			{
				errorCode = INV_ALARM;
				return errorCode;
			}
			if (tokenPropDescrip == NULL)
			{
				errorCode = INV_ALARM;
				return errorCode;
			}
			tempProperty = initializeProperty(tokenPropDescrip, token);
			if (tempProperty == NULL)
			{
				errorCode = OTHER_ERROR;
				return errorCode;
			}
			insertBack(alarm->properties, (void *)tempProperty);
		}
	}

	errorCode = INV_ALARM;
	return errorCode;
}

/** Function to delete all calendar content and free all the memory.
 *@pre Calendar object exists, is not null, and has not been freed
 *@post Calendar object had been freed
 *@return none
 *@param obj - a pointer to a Calendar struct
**/
void deleteCalendar(Calendar *obj)
{
	if (!obj)
	{
		return;
	}

	freeList(obj->events);
	freeList(obj->properties);
	free(obj);
}

/** Function to create a string representation of a Calendar object.
 *@pre Calendar object exists, is not null, and is valid
 *@post Calendar has not been modified in any way, and a string representing the Calendar contents has been created
 *@return a string contaning a humanly readable representation of a Calendar object
 *@param obj - a pointer to a Calendar struct
**/
char *printCalendar(const Calendar *obj)
{
	char *calendarStr;
	char *eventStr;
	char *propStr;
	char tempStr[250];
	int len = 0;
	int totalLen = 0;

	if (!obj)
	{
		calendarStr = malloc(sizeof(char) * (strlen("Error in printCalendar") + 1));
		sprintf(calendarStr, "Error in printCalendar");
		return calendarStr;
	}

	//printf("in printfCalendar function\n");
	eventStr = toString(obj->events);
	propStr = toString(obj->properties);
	len = sprintf(tempStr, "CALENDAR: version:%.2f, prodid:%s", obj->version, obj->prodID);
	totalLen = len + strlen(eventStr) + strlen(propStr);
	calendarStr = malloc(sizeof(char) * (totalLen + 110));
	strcpy(calendarStr, tempStr);

	if (strlen(eventStr) != 0)
	{
		strcat(calendarStr, eventStr);
	}
	if (strlen(propStr) != 0)
	{
		strcat(calendarStr, "\nCalendar Properties:");
		strcat(calendarStr, propStr);
	}

	free(propStr);
	free(eventStr);

	return calendarStr;
}

/** Function to "convert" the ICalErrorCode into a humanly redabale string.
 *@return a string contaning a humanly readable representation of the error code by indexing into
          the descr array using rhe error code enum value as an index
 *@param err - an error code
**/
char *printError(ICalErrorCode err)
{
	char tempStr[200];
	char *strError;

	switch (err)
	{
	case OK:
		strcpy(tempStr, "OK");
		break;
	case INV_FILE:
		strcpy(tempStr, "The filename is invalid. It should end by .ics");
		break;
	case INV_CAL:
		strcpy(tempStr, "Invalid Calendar");
		break;
	case INV_VER:
		strcpy(tempStr, "Calendar Version is invalid.");
		break;
	case DUP_VER:
		strcpy(tempStr, "Multiple Calendar Versions");
		break;
	case INV_PRODID:
		strcpy(tempStr, "Product ID is invalid.");
		break;
	case DUP_PRODID:
		strcpy(tempStr, "Multiple Calendar PRODID");
		break;
	case INV_EVENT:
		strcpy(tempStr, "Missing Event Information..");
		break;
	case INV_DT:
		strcpy(tempStr, "Date/time Format is invalid.");
		break;
	case INV_ALARM:
		strcpy(tempStr, "Invalid Alarm Component");
		break;
	case WRITE_ERROR:
		strcpy(tempStr, "The file was not saved.");
		break;
	case OTHER_ERROR:
		strcpy(tempStr, "Other Error");
		break;
	default:
		strcpy(tempStr, "Unknown Error");
		break;
	}

	strError = malloc(sizeof(char) * (strlen(tempStr) + 1));
	strcpy(strError, tempStr);

	return strError;
}

/** Function to writing a Calendar object into a file in iCalendar format.
 *@pre Calendar object exists, is not null, and is valid
 *@post Calendar has not been modified in any way, and a file representing the
        Calendar contents in iCalendar format has been created
 *@return the error code indicating success or the error encountered when parsing the calendar
 *@param obj - a pointer to a Calendar struct
 **/
ICalErrorCode writeCalendar(char *fileName, const Calendar *obj)
{
	ICalErrorCode errorCode;
	FILE *fp;
	ListIterator iter;
	void *element;

	errorCode = WRITE_ERROR;

	if (obj == NULL)
	{
		return errorCode;
	}
	if (fileName == NULL || strlen(fileName) == 0)
	{
		return errorCode;
	}

	//CHECK IF FILE ENDS WITH CORRECT EXTENSION

	fp = fopen(fileName, "w");
	if (fp == NULL)
	{
		return errorCode;
	}

	if (fprintf(fp, "%s\r\n", "BEGIN:VCALENDAR") < 0)
	{
		return errorCode;
	}

	if (fprintf(fp, "VERSION:%.1f\r\nPRODID:%s\r\n", obj->version, obj->prodID) < 0)
	{
		return errorCode;
	}

	iter = createIterator(obj->events);

	while ((element = nextElement(&iter)) != NULL)
	{
		Event *tempEvent = (Event *)element;
		errorCode = writeEvent(fp, tempEvent);
		if (errorCode == WRITE_ERROR)
		{
			return errorCode;
		}
	}

	iter = createIterator(obj->properties);

	while ((element = nextElement(&iter)) != NULL)
	{
		Property *tempProperty = (Property *)element;
		errorCode = writeProperty(fp, tempProperty);
		if (errorCode == WRITE_ERROR)
		{
			return errorCode;
		}
	}

	if (fprintf(fp, "%s\r\n", "END:VCALENDAR") < 0)
	{
		return errorCode;
	}

	fclose(fp);
	errorCode = OK;
	return errorCode;
}

ICalErrorCode writeEvent(FILE *fp, const Event *event)
{
	ICalErrorCode errorCode = WRITE_ERROR;
	ListIterator iter;
	void *element;

	//write start of event
	if ((fprintf(fp, "%s\r\n", "BEGIN:VEVENT")) < 0)
	{
		return errorCode;
	}
	//write UID
	if (fprintf(fp, "UID:%s\r\n", event->UID) < 0)
	{
		return errorCode;
	}
	//write DTSTAMP
	if ((writeDateTime(fp, event->creationDateTime, 0)) == WRITE_ERROR)
	{
		return errorCode;
	}
	//write DTSTART
	if ((writeDateTime(fp, event->startDateTime, 1)) == WRITE_ERROR)
	{
		return errorCode;
	}
	//write alarms
	iter = createIterator(event->alarms);
	while ((element = nextElement(&iter)) != NULL)
	{
		Alarm *tempAlarm = (Alarm *)element;
		errorCode = writeAlarm(fp, tempAlarm);
		if (errorCode == WRITE_ERROR)
		{
			return errorCode;
		}
	}

	//write event properties
	iter = createIterator(event->properties);
	while ((element = nextElement(&iter)) != NULL)
	{
		Property *tempProperty = (Property *)element;
		errorCode = writeProperty(fp, tempProperty);
		if (errorCode == WRITE_ERROR)
		{
			return errorCode;
		}
	}

	//write end of event
	if ((fprintf(fp, "%s\r\n", "END:VEVENT")) < 0)
	{
		return errorCode;
	}

	errorCode = OK;
	return errorCode;
}

// 0 for dtstamp and 1 for dtstart
ICalErrorCode writeDateTime(FILE *fp, const DateTime tempDT, int type)
{
	//tempDT.date, tempDT.time, tempDT.UTC
	ICalErrorCode errorCode = WRITE_ERROR;

	if (type == 0)
	{
		if (tempDT.UTC == true)
		{
			if ((fprintf(fp, "DTSTAMP:%sT%sZ\r\n", tempDT.date, tempDT.time)) < 0)
			{
				return errorCode;
			}
		}
		else
		{
			if ((fprintf(fp, "DTSTAMP:%sT%s\r\n", tempDT.date, tempDT.time)) < 0)
			{
				return errorCode;
			}
		}
	}
	else if (type == 1)
	{
		if (tempDT.UTC == true)
		{
			if ((fprintf(fp, "DTSTART:%sT%sZ\r\n", tempDT.date, tempDT.time)) < 0)
			{
				return errorCode;
			}
		}
		else
		{
			if ((fprintf(fp, "DTSTART:%sT%s\r\n", tempDT.date, tempDT.time)) < 0)
			{
				return errorCode;
			}
		}
	}

	errorCode = OK;
	return errorCode;
}

ICalErrorCode writeAlarm(FILE *fp, const Alarm *tempAlarm)
{
	//action, trigger, list of properties
	ICalErrorCode errorCode = WRITE_ERROR;
	ListIterator iter;
	void *element;

	if (fprintf(fp, "%s\r\n", "BEGIN:VALARM") < 0)
	{
		return errorCode;
	}

	if (fprintf(fp, "ACTION:%s\r\nTRIGGER:%s\r\n", tempAlarm->action, tempAlarm->trigger) < 0)
	{
		return errorCode;
	}

	iter = createIterator(tempAlarm->properties);
	while ((element = nextElement(&iter)) != NULL)
	{
		Property *tempProperty = (Property *)element;
		errorCode = writeProperty(fp, tempProperty);
		if (errorCode == WRITE_ERROR)
		{
			return errorCode;
		}
	}

	if (fprintf(fp, "%s\r\n", "END:VALARM") < 0)
	{
		return errorCode;
	}

	errorCode = OK;
	return errorCode;
}

ICalErrorCode writeProperty(FILE *fp, const Property *tempProp)
{
	//propName, propDescr
	ICalErrorCode errorCode = WRITE_ERROR;

	if (fprintf(fp, "%s:%s\r\n", tempProp->propName, tempProp->propDescr) < 0)
	{
		return errorCode;
	}

	errorCode = OK;
	return errorCode;
}

/** Function to validating an existing a Calendar object
 *@pre Calendar object exists and is not null
 *@post Calendar has not been modified in any way
 *@return the error code indicating success or the error encountered when validating the calendar
 *@param obj - a pointer to a Calendar struct
 **/
ICalErrorCode validateCalendar(const Calendar *obj)
{
	ICalErrorCode errorCode;
	errorCode = OK;
	ListIterator iter;
	void *element;
	bool calscale = false;
	bool method = false;

	if (obj == NULL)
	{
		return INV_CAL;
	}

	//Cal struct
	// prodID not exceed, events not null, properties not null
	if (strlen(obj->prodID) == 0 || obj->events == NULL || obj->properties == NULL)
	{
		errorCode = INV_CAL;
		return errorCode;
	}

	//Check if list of events is >= 1 then loop through list and validate each Event struct
	if (getLength(obj->events) < 1)
	{
		errorCode = INV_CAL;
		return errorCode;
	}
	else
	{
		iter = createIterator(obj->events);
		while ((element = nextElement(&iter)) != NULL)
		{
			Event *tempEvent = (Event *)element;
			errorCode = validateEvent(tempEvent);
			if (errorCode != OK)
			{
				return errorCode;
			}
		}
	}

	//If list is not empty, loop through list and validate each properties
	if (getLength(obj->properties) != 0)
	{
		iter = createIterator(obj->properties);
		while ((element = nextElement(&iter)) != NULL)
		{
			Property *tempProp = (Property *)element;

			if (strcmp(tempProp->propName, "CALSCALE") == 0 && !calscale)
			{
				calscale = true;
				if (strlen(tempProp->propDescr) == 0)
				{
					return INV_CAL;
				}
			}
			else if (strcmp(tempProp->propName, "METHOD") == 0 && !method)
			{
				method = true;
				if (strlen(tempProp->propDescr) == 0)
				{
					return INV_CAL;
				}
			}
			else
			{
				return INV_CAL;
			}
		}
	}
	return errorCode;
}

//list of event list
//iterate through list and validate each event
ICalErrorCode validateEvent(Event *event)
{
	int uidLen = 0;
	ListIterator iter;
	ListIterator iter2;
	void *element;
	void *elem;
	bool dtend = false;
	bool duration = false;
	int dup = 0;

	uidLen = strlen(event->UID);
	if (uidLen > 1000 || uidLen <= 0)
	{
		return INV_EVENT;
	}

	if (strlen(event->creationDateTime.date) != 8 || strlen(event->creationDateTime.time) != 6 || strlen(event->startDateTime.date) != 8 || strlen(event->startDateTime.time) != 6)
	{
		return INV_EVENT;
	}

	if (event->properties == NULL || event->alarms == NULL)
	{
		return INV_EVENT;
	}

	if (getLength(event->alarms) != 0)
	{
		iter = createIterator(event->alarms);
		while ((element = nextElement(&iter)) != NULL)
		{
			Alarm *tempAlarm = (Alarm *)element;
			ICalErrorCode errorCode = validateAlarm(tempAlarm);
			if (errorCode != OK)
			{
				return errorCode;
			}
		}
	}

	if (getLength(event->properties) != 0)
	{
		iter = createIterator(event->properties);
		while ((element = nextElement(&iter)) != NULL)
		{
			Property *tempProp = (Property *)element;

			if (strcmp(tempProp->propName, "CLASS") == 0 || strcmp(tempProp->propName, "CREATED") == 0 || strcmp(tempProp->propName, "DESCRIPTION") == 0 || strcmp(tempProp->propName, "GEO") == 0 || strcmp(tempProp->propName, "LAST-MODIFIED") == 0 || strcmp(tempProp->propName, "LOCATION") == 0 || strcmp(tempProp->propName, "ORGANIZER") == 0 || strcmp(tempProp->propName, "PRIORITY") == 0 || strcmp(tempProp->propName, "SEQUENCE") == 0 || strcmp(tempProp->propName, "STATUS") == 0 || strcmp(tempProp->propName, "SUMMARY") == 0 || strcmp(tempProp->propName, "TRANSP") == 0 || strcmp(tempProp->propName, "URL") == 0 || strcmp(tempProp->propName, "RECURRENCE-ID") == 0 || strcmp(tempProp->propName, "RRULE") == 0 || strcmp(tempProp->propName, "DTEND") == 0 || strcmp(tempProp->propName, "DURATION") == 0)
			{
				if (strcmp(tempProp->propDescr, "") == 0)
				{
					return INV_EVENT;
				}
				dup = 0;
				iter2 = createIterator(event->properties);
				while ((elem = nextElement(&iter2)) != NULL)
				{
					Property *compareProp = (Property *)elem;
					if ((compareProperties((void *)tempProp, (void *)compareProp)) == 0)
					{
						dup++;
					}
				}
				if (dup > 1)
				{ //means that this property occurs twice in the list
					return INV_EVENT;
				}

				if (strcmp(tempProp->propName, "DTEND") == 0)
				{
					dtend = true;
				}

				if (strcmp(tempProp->propName, "DURATION") == 0)
				{
					duration = true;
				}
			}
			else if (strcmp(tempProp->propName, "ATTACH") == 0 || strcmp(tempProp->propName, "ATTENDEE") == 0 || strcmp(tempProp->propName, "CATEGORIES") == 0 || strcmp(tempProp->propName, "COMMENT") == 0 || strcmp(tempProp->propName, "CONTACT") == 0 || strcmp(tempProp->propName, "EXDATE") == 0 || strcmp(tempProp->propName, "REQUEST_STATUS") == 0 || strcmp(tempProp->propName, "RELATED-TO") == 0 || strcmp(tempProp->propName, "RESOURCES") == 0 || strcmp(tempProp->propName, "RDATE") == 0)
			{
				if (strcmp(tempProp->propDescr, "") == 0)
				{
					return INV_EVENT;
				}
			}
			else
			{
				return INV_EVENT;
			}
		}

		if (duration && dtend)
		{
			return INV_EVENT;
		}
	}
	return OK;
}

ICalErrorCode validateAlarm(Alarm *alarm)
{
	ListIterator iter;
	ListIterator iter2;
	void *element;
	void *elem;
	bool duration = false;
	bool repeat = false;
	int dup = 0;
	int actionLen = strlen(alarm->action);
	if (actionLen > 200 || actionLen == 0)
	{
		return INV_ALARM;
	}
	if (alarm->trigger != NULL)
	{
		if (strlen(alarm->trigger) == 0)
		{
			return INV_ALARM;
		}
	}
	else
	{
		return INV_ALARM;
	}
	if (alarm->properties == NULL)
	{
		return INV_ALARM;
	}

	if (getLength(alarm->properties) != 0)
	{
		iter = createIterator(alarm->properties);
		while ((element = nextElement(&iter)) != NULL)
		{
			Property *tempProp = (Property *)element;

			if (strcmp(tempProp->propName, "DURATION") == 0 || strcmp(tempProp->propName, "REPEAT") == 0 || strcmp(tempProp->propName, "ATTACH") == 0)
			{
				if (strcmp(tempProp->propDescr, "") == 0)
				{
					return INV_ALARM;
				}
				dup = 0;
				iter2 = createIterator(alarm->properties);
				while ((elem = nextElement(&iter2)) != NULL)
				{
					Property *tempCompare = (Property *)elem;
					if ((compareProperties((void *)tempCompare, (void *)tempProp)) == 0)
					{
						dup++;
					}
				}
				if (dup > 1)
				{
					return INV_ALARM;
				}
				if (strcmp(tempProp->propName, "DURATION") == 0)
				{
					duration = true;
				}
				if (strcmp(tempProp->propName, "REPEAT") == 0)
				{
					repeat = true;
				}
			}
			else
			{
				return INV_ALARM;
			}
		}
		if ((duration && !repeat) || (!duration && repeat))
		{
			return INV_ALARM;
		}
	}

	return OK;
}

/** Function to converting a DateTime into a JSON string
 *@pre N/A
 *@post DateTime has not been modified in any way
 *@return A string in JSON format
 *@param prop - a DateTime struct
 **/
//{\"date\":\"date val\",\"time\":\"time val\",\"isUTC\":utcVal}  //30
//e.g {"date":"19540203","time":"123012","isUTC":true}
char *dtToJSON(DateTime prop)
{
	char *toReturn;
	int len = 0;

	len = 30 + 10 + strlen(prop.date) + strlen(prop.time) + 1;
	toReturn = malloc(sizeof(char) * len);
	if (prop.UTC)
	{ //if UTC is true
		if ((sprintf(toReturn, "{\"date\":\"%s\",\"time\":\"%s\",\"isUTC\":%s}", prop.date, prop.time, "true")) < 0)
		{
			return NULL;
		}
	}
	else
	{
		if ((sprintf(toReturn, "{\"date\":\"%s\",\"time\":\"%s\",\"isUTC\":%s}", prop.date, prop.time, "false")) < 0)
		{
			return NULL;
		}
	}

	return toReturn;
}

/** Function to converting an Event into a JSON string
 *@pre Event is not NULL
 *@post Event has not been modified in any way
 *@return A string in JSON format
 *@param event - a pointer to an Event struct
 **/
//{\"startDT\":DTval,\"numProps\":propVal,\"numAlarms\":almVal,\"summary\":\"sumVal\"}
char *eventToJSON(const Event *event)
{
	char *toReturn;
	int len = 0;
	char *startDT;
	char *sumVal;
	int propVal = 0;
	int numAlarms = 0;
	ListIterator iter;
	int i = 0;
	int escapeSeq = 0;
	void *element;
	int match = 0;
	bool summaryPresent = false;

	if (event == NULL)
	{
		len = strlen("{}") + 1;
		toReturn = malloc(sizeof(char) * len);
		strcpy(toReturn, "{}");
		return toReturn;
	}

	//string which contains startDateTime content in Json format
	startDT = dtToJSON(event->startDateTime);

	// propVal is 3 required properties + num of properties in list
	propVal = getLength(event->properties) + 3;
	numAlarms = getLength(event->alarms);

	//Find if summary is in list of properties
	if (propVal > 3)
	{
		Property *tempProp = initializeProperty("Temp", "SUMMARY");
		iter = createIterator((List *)event->properties);
		while ((element = nextElement(&iter)) != NULL)
		{
			match = compareProperties(element, (void *)tempProp);
			if (match == 0)
			{ //summary is present
				i = 0;
				Property *tempElement = (Property *)element;
				//check if summary has any " in it
				for (i = 0; i < strlen(tempElement->propDescr); i++)
				{
					if (tempElement->propDescr[i] == '"')
					{
						escapeSeq++;
					}
				}
				//if (escapeSeq == 0) {    //" is not present
				sumVal = malloc(sizeof(char) * (strlen(tempElement->propDescr) + 1));
				strcpy(sumVal, tempElement->propDescr);
				summaryPresent = true;
			}
		}
		if (summaryPresent == false)
		{ //if summary not present in list of properties
			sumVal = malloc(sizeof(char));
			strcpy(sumVal, "");
		}
		free(tempProp);
	}
	else
	{ //if theres only the 3 required properties
		sumVal = malloc(sizeof(char));
		strcpy(sumVal, "");
	}

	len = strlen(startDT) + strlen(sumVal) + 51 + 40;
	toReturn = malloc(sizeof(char) * len);
	sprintf(toReturn, "{\"startDT\":%s,\"numProps\":%d,\"numAlarms\":%d,\"summary\":\"%s\"}", startDT, propVal, numAlarms, sumVal);

	free(startDT);
	free(sumVal);
	return toReturn;
}

/** Function to converting an Event list into a JSON string
 *@pre Event list is not NULL
 *@post Event list has not been modified in any way
 *@return A string in JSON format
 *@param eventList - a pointer to an Event list
 **/
/* [{"startDT":{"date":"19540203","time":"123012","isUTC":true},"numProps":3,"numAlarms":
0,"summary":""},{"startDT":{"date":"19540203","time":"123012","isUTC":true},"numProps":
4,"numAlarms":2,"summary":"Do taxes"}]*/
char *eventListToJSON(const List *eventList)
{
	int eventListLen = 0;
	char *toReturn;
	int len = 0;
	ListIterator iter;
	char **eventJson;
	int i = 0;
	void *element;

	if (eventList == NULL)
	{
		len = strlen("[]") + 1;
		toReturn = malloc(sizeof(char) * len);
		strcpy(toReturn, "[]");
		return toReturn;
	}

	eventListLen = getLength((List *)eventList);
	if (eventListLen == 0)
	{
		len = strlen("[]") + 1;
		toReturn = malloc(sizeof(char) * len);
		strcpy(toReturn, "[]");
		return toReturn;
	}

	eventJson = malloc(sizeof(char *) * eventListLen);
	iter = createIterator((List *)eventList);
	while ((element = nextElement(&iter)) != NULL)
	{
		eventJson[i] = eventToJSON((Event *)element);
		len = len + strlen(eventJson[i]);
		i++;
	}

	toReturn = malloc(sizeof(char) * (len + 3 + i - 1));
	strcpy(toReturn, "[");

	for (i = 0; i < eventListLen; i++)
	{
		strcat(toReturn, eventJson[i]);
		if (eventListLen > 1 && (i < (eventListLen - 1)))
		{
			strcat(toReturn, ",");
		}
		free(eventJson[i]);
	}
	strcat(toReturn, "]");
	free(eventJson);

	return toReturn;
}

/** Function to converting a Calendar into a JSON string
 *@pre Calendar is not NULL
 *@post Calendar has not been modified in any way
 *@return A string in JSON format
 *@param cal - a pointer to a Calendar struct
 **/
//{\"version\":2,\"prodID\":\"-//hacksw/handcal//NONSGML v1.0//EN\",\"numProps\":2,\"numEvents\":1}
// if cal is null, returns {}
char *calendarToJSON(const Calendar *cal)
{
	char *toReturn;
	int numProps = 0;
	int numEvents = 0;
	int len = 0;

	if (cal == NULL)
	{
		len = strlen("{}") + 1;
		toReturn = malloc(sizeof(char) * len);
		strcpy(toReturn, "{}");
		return toReturn;
	}

	numProps = getLength(cal->properties) + 2;
	numEvents = getLength(cal->events);

	if (numEvents == 0 || cal->events == NULL)
	{
		toReturn = malloc(sizeof(char) * (strlen("{}") + 1));
		strcpy(toReturn, "{}");
		return toReturn;
	}

	len = strlen(cal->prodID) + 60 + 50;
	toReturn = malloc(sizeof(char) * len);
	sprintf(toReturn, "{\"version\":%d,\"prodID\":\"%s\",\"numProps\":%d,\"numEvents\":%d}", (int)cal->version, cal->prodID, numProps, numEvents);

	return toReturn;
}

/** Function to converting a JSON string into a Calendar struct
 *@pre JSON string is not NULL
 *@post String has not been modified in any way
 *@return A newly allocated and partially initialized Calendar struct
 *@param str - a pointer to a string
 **/
// {"version":2,"prodID":"-//hacksw/handcal//NONSGML v1.0//EN"}
// 0 to 11    , + 11 chars
Calendar *JSONtoCalendar(const char *str)
{
	char tempStr[200];
	int i = 0;
	int j = 0;
	int counter = 0;
	char version[30];
	char prodID[1000];

	if (str == NULL)
	{
		return NULL;
	}

	strncpy(tempStr, str, 11);
	tempStr[11] = '\0';

	if (strcmp(tempStr, "{\"version\":") != 0)
	{
		return NULL;
	}
	else
	{
		tempStr[0] = '\0';
	}

	for (i = 11; i < strlen(str) - 2; i++)
	{
		if (str[i] != ',')
		{
			version[j] = str[i];
			j++;
		}
		else
		{
			counter = i;
			break;
		}
	}

	if (j == 0)
	{
		return NULL;
	}

	strncpy(tempStr, &str[counter], 11);
	tempStr[11] = '\0';

	if (strcmp(tempStr, ",\"prodID\":\"") != 0)
	{
		return NULL;
	}
	else
	{
		counter = counter + 11;
	}
	j = 0;
	for (i = counter; i < (strlen(str) - 2); i++)
	{
		prodID[j] = str[i];
		j++;
	}
	prodID[j] = '\0';
	if (j == 0)
	{
		return NULL;
	}

	if (str[strlen(str) - 1] != '}' || str[strlen(str) - 2] != '"')
	{
		return NULL;
	}

	Calendar *tempObj = malloc(sizeof(Calendar));
	tempObj->version = atof(version);
	strcpy(tempObj->prodID, prodID);
	tempObj->events = initializeList(&printEvent, &deleteEvent, &compareEvents);
	tempObj->properties = initializeList(&printProperty, &deleteProperty, &compareProperties);

	return tempObj;
}

/** Function to converting a JSON string into an Event struct
 *@pre JSON string is not NULL
 *@post String has not been modified in any way
 *@return A newly allocated and partially initialized Event struct
 *@param str - a pointer to a string
 **/
//{"UID":"1234"}
Event *JSONtoEvent(const char *str)
{
	char tempStr[200];
	int i = 0;
	int j = 0;
	char uid[1000];

	if (str == NULL)
	{
		return NULL;
	}

	strncpy(tempStr, str, 8);
	tempStr[8] = '\0';
	if (strcmp(tempStr, "{\"UID\":\"") != 0)
	{
		return NULL;
	}

	for (i = 8; i < (strlen(str) - 2); i++)
	{
		uid[j] = str[i];
		j++;
	}
	uid[j] = '\0';

	if (str[strlen(str) - 1] != '}' || str[strlen(str) - 2] != '"')
	{
		return NULL;
	}

	Event *tempEvent = malloc(sizeof(Event));
	strcpy(tempEvent->UID, uid);
	tempEvent->properties = initializeList(&printProperty, &deleteProperty, &compareProperties);
	tempEvent->alarms = initializeList(&printAlarm, &deleteAlarm, &compareAlarms);

	return tempEvent;
}

/** Function to adding an Event struct to an existing Calendar struct
 *@pre arguments are not NULL
 *@post The new event has been added to the calendar's events list
 *@return N/A
 *@param cal - a Calendar struct
 *@param toBeAdded - an Event struct
 **/
void addEvent(Calendar *cal, Event *toBeAdded)
{
	if (cal == NULL || toBeAdded == NULL)
	{
		return;
	}

	insertBack(cal->events, (void *)toBeAdded);
}

// ************* List helper functions - MUST be implemented ***************
void deleteEvent(void *toBeDeleted)
{
	Event *tempEvent;

	if (toBeDeleted == NULL)
	{
		return;
	}

	tempEvent = (Event *)toBeDeleted;
	freeList(tempEvent->properties);
	freeList(tempEvent->alarms);
	free(tempEvent);
}

int compareEvents(const void *first, const void *second)
{
	return 0;
}

char *printEvent(void *toBePrinted)
{
	char *returnStr;
	char *propStr;
	char *alarmStr;
	char *datetimeStr1;
	char *datetimeStr2;

	if (toBePrinted == NULL)
	{
		returnStr = malloc(sizeof(char));
		strcpy(returnStr, "");
		return returnStr;
	}

	Event *event;
	event = (Event *)toBePrinted;
	propStr = toString(event->properties);
	alarmStr = toString(event->alarms);
	datetimeStr1 = printDate((void *)&event->creationDateTime);
	datetimeStr2 = printDate((void *)&event->startDateTime);
	int len = strlen(event->UID) + strlen(propStr) + strlen(alarmStr) + strlen(datetimeStr1) + strlen(datetimeStr2) + 500;
	returnStr = malloc(sizeof(char) * len);
	sprintf(returnStr, "EVENT:\nUID:%s\n", event->UID);
	strcat(returnStr, datetimeStr1);
	strcat(returnStr, datetimeStr2);
	if (strlen(propStr) != 0)
	{
		strcat(returnStr, "Event properties:");
		strcat(returnStr, propStr);
	}
	if (strlen(alarmStr) != 0)
	{
		strcat(returnStr, alarmStr);
	}

	free(propStr);
	free(alarmStr);

	free(datetimeStr1);
	free(datetimeStr2);

	return returnStr;
}

void deleteAlarm(void *toBeDeleted)
{
	Alarm *tempAlarm;

	if (!toBeDeleted)
	{
		return;
	}

	tempAlarm = (Alarm *)toBeDeleted;
	if (tempAlarm->trigger != NULL)
	{
		free(tempAlarm->trigger);
	}
	freeList(tempAlarm->properties);
	free(tempAlarm);
}

int compareAlarms(const void *first, const void *second)
{

	int value = 0;

	value = 1;
	return value;
}

char *printAlarm(void *toBePrinted)
{
	char *returnStr;
	char *propertyStr;

	Alarm *alarm;
	if (toBePrinted == NULL)
	{
		returnStr = malloc(sizeof(char));
		strcpy(returnStr, "");
		return returnStr;
	}

	alarm = (Alarm *)toBePrinted;
	propertyStr = toString(alarm->properties);
	int len = strlen(alarm->action) + strlen(alarm->trigger) + strlen(propertyStr) + 500;
	returnStr = malloc(sizeof(char) * len);
	sprintf(returnStr, "ALARM:\nAction: %s, Trigger: %s\nAlarm properties: ", alarm->action, alarm->trigger);

	if (strlen(propertyStr) != 0)
	{
		strcat(returnStr, propertyStr);
	}

	free(propertyStr);

	return returnStr;
}

void deleteProperty(void *toBeDeleted)
{
	Property *tempProp;
	if (toBeDeleted == NULL)
	{
		return;
	}

	tempProp = (Property *)toBeDeleted;
	free(tempProp);
}

int compareProperties(const void *first, const void *second)
{
	Property *tempFirst;
	Property *tempSecond;
	int value = 0;

	tempFirst = (Property *)first;
	tempSecond = (Property *)second;

	value = strcmp(tempFirst->propName, tempSecond->propName);

	return value;
}
char *printProperty(void *toBePrinted)
{
	char *returnStr;
	Property *property;
	if (toBePrinted == NULL)
	{
		returnStr = malloc(sizeof(char));
		strcpy(returnStr, "");
		return returnStr;
	}
	property = (Property *)toBePrinted;
	returnStr = malloc(sizeof(char) * (strlen(property->propName) + strlen(property->propDescr) + 100));
	sprintf(returnStr, "%s: %s", property->propName, property->propDescr);

	return returnStr;
}

void deleteDate(void *toBeDeleted)
{
	/*DateTime tempDateTime;
	if (toBeDelete == NULL) {
		return;
	}

	tempDateTime = (DateTime)toBeDeleted;
	freeList(tempDateTime);*/
}

int compareDates(const void *first, const void *second)
{
	return 0;
}

char *printDate(void *toBePrinted)
{
	char *returnStr;
	int len = 0;

	if (toBePrinted == NULL)
	{
		returnStr = malloc(sizeof(char));
		strcpy(returnStr, "");
		return returnStr;
	}
	DateTime *tempDateTime;

	tempDateTime = (DateTime *)toBePrinted;
	len = strlen(tempDateTime->date) + strlen(tempDateTime->time) + 300;
	returnStr = malloc(sizeof(char) * len);
	sprintf(returnStr, "Date: %s, Time: %s\n", tempDateTime->date, tempDateTime->time);

	return returnStr;
}

/**** ADDITIONAL FUNCTIONS FOR A3 ********/
//{\"action\":\"audio val\",\"trigger\":\"trigger val\",\"numProps\":propVal}
char *alarmToJSON(const Alarm *alarm)
{
	char *toReturn;
	int len = 0;
	int propVal = 0;

	if (alarm == NULL)
	{
		len = strlen("{}") + 1;
		toReturn = malloc(sizeof(char) * len);
		strcpy(toReturn, "{}");
		return toReturn;
	}

	len = 42 + 20 + strlen(alarm->action) + strlen(alarm->trigger);
	propVal = getLength(alarm->properties) + 2;

	toReturn = malloc(sizeof(char) * len);
	if ((sprintf(toReturn, "{\"action\":\"%s\",\"trigger\":\"%s\",\"numProps\":%d}", alarm->action, alarm->trigger, propVal)) < 0)
	{
		return NULL;
	}

	return toReturn;
}

//returns in the format of [{alarm1},{alarm2}]
// or [] if list is null or list is empty
char *alarmListToJSON(const List *alarmList)
{
	int alarmListLen = 0;
	char *toReturn;
	int len = 0;
	ListIterator iter;
	char **alarmJson;
	int i = 0;
	void *element;

	if (alarmList == NULL)
	{
		len = strlen("[]") + 1;
		toReturn = malloc(sizeof(char) * len);
		strcpy(toReturn, "[]");
		return toReturn;
	}
	alarmListLen = getLength((List *)alarmList);
	if (alarmListLen == 0)
	{
		len = strlen("[]") + 1;
		toReturn = malloc(sizeof(char) * len);
		strcpy(toReturn, "[]");
		return toReturn;
	}

	alarmJson = malloc(sizeof(char *) * alarmListLen);
	iter = createIterator((List *)alarmList);
	while ((element = nextElement(&iter)) != NULL)
	{
		alarmJson[i] = alarmToJSON((Alarm *)element);
		len = len + strlen(alarmJson[i]);
		i++;
	}

	toReturn = malloc(sizeof(char) * (len + 3 + i - 1));
	strcpy(toReturn, "[");

	for (i = 0; i < alarmListLen; i++)
	{
		strcat(toReturn, alarmJson[i]);
		if (alarmListLen > 1 && (i < (alarmListLen - 1)))
		{
			strcat(toReturn, ",");
		}
		free(alarmJson[i]);
	}
	strcat(toReturn, "]");
	free(alarmJson);

	return toReturn;
}

//{\"propName\":\"propNameVal\",\"propDescr\":\"propDescrVal\"} //30
char *PropertyToJSON(const Property *prop)
{
	char *toReturn;
	int len = 0;

	if (prop == NULL)
	{
		len = strlen("{}") + 1;
		toReturn = malloc(sizeof(char) * len);
		strcpy(toReturn, "{}");
		return toReturn;
	}

	len = 32 + strlen(prop->propName) + strlen(prop->propDescr);
	toReturn = malloc(sizeof(char) * len);
	if ((sprintf(toReturn, "{\"propName\":\"%s\",\"propDescr\":\"%s\"}", prop->propName, prop->propDescr)) < 0)
	{
		return NULL;
	}

	return toReturn;
}
//[{prop1},{prop2}]
// [] is empty or propList is null(unlikely tho)
char *propListToJSON(const List *propList)
{
	int propListLen = 0;
	char *toReturn;
	int len = 0;
	ListIterator iter;
	int i = 0;
	void *element;
	char **propJson;

	if (propList == NULL)
	{
		len = strlen("[]") + 1;
		toReturn = malloc(sizeof(char) * len);
		strcpy(toReturn, "[]");
		return toReturn;
	}

	propListLen = getLength((List *)propList);
	if (propListLen == 0)
	{
		len = strlen("[]") + 1;
		toReturn = malloc(sizeof(char) * len);
		strcpy(toReturn, "[]");
		return toReturn;
	}

	propJson = malloc(sizeof(char *) * propListLen);
	iter = createIterator((List *)propList);
	while ((element = nextElement(&iter)) != NULL)
	{
		propJson[i] = PropertyToJSON((Property *)element);
		len = len + strlen(propJson[i]);
		i++;
	}

	toReturn = malloc(sizeof(char) * (len + 3 + i - 1));
	strcpy(toReturn, "[");

	for (i = 0; i < propListLen; i++)
	{
		strcat(toReturn, propJson[i]);
		if (propListLen > 1 && (i < (propListLen - 1)))
		{
			strcat(toReturn, ",");
		}
		free(propJson[i]);
	}
	strcat(toReturn, "]");
	free(propJson);

	return toReturn;
}

char* checkFileValidity(char* filename) {
	ICalErrorCode error;
	Calendar* cal;
	char* toReturn;

	error = createCalendar(filename, &cal);
	if (error == OK) {
		error = validateCalendar(cal);
	}
	if (error != OK) {
		cal = NULL;
		deleteCalendar(cal);
		toReturn = printError(error);
		return toReturn;
	} else {
		deleteCalendar(cal);
		toReturn = printError(error);
		return toReturn;
	}
}

char *getCalJSON(char *filename)
{
	Calendar *cal;
	ICalErrorCode error;

	error = createCalendar(filename, &cal);
	if (error != OK) {
		cal = NULL;
	}

	char *calJSON = calendarToJSON(cal);
	deleteCalendar(cal);
	return calJSON;
}

//We assume that all files should be valid
char *getEventListJSON(char *filename)
{
	Calendar *cal;
	ICalErrorCode error;

	error = createCalendar(filename, &cal);

	if (error != OK)
	{
		int len = strlen("[]") + 1;
		char *toReturn = malloc(sizeof(char) * len);
		strcpy(toReturn, "[]");
		return toReturn;
	}
	else
	{
		char *eventListJSON = eventListToJSON(cal->events);
		deleteCalendar(cal);
		return eventListJSON;
	}
}

char *getAlarmListJSON(char *filename, int eventNo) {
	Calendar *cal;
	ICalErrorCode error;
	int i = 0;
	void *element;
	ListIterator iter;

	error = createCalendar(filename, &cal);

	if (error != OK) {
		int len = strlen("[]") + 1;
		char *toReturn = malloc(sizeof(char) * len);
		strcpy(toReturn, "[]");
		return toReturn;
	}
	else
	{
		iter = createIterator((List *)cal->events);
		while ((element = nextElement(&iter)) != NULL) {
			if (i == eventNo) {
				Event *event = (Event *)element;
				char *alarmJSON = alarmListToJSON(event->alarms);
				deleteCalendar(cal);
				return alarmJSON;
			}
			else {
				i++;
			}
		}
	}
	return NULL;
}

char *getPropListJSON(char *filename, int eventNo) {
	Calendar *cal;
	ICalErrorCode error;
	int i = 0;
	void *element;
	ListIterator iter;

	error = createCalendar(filename, &cal);

	if (error != OK) {
		int len = strlen("[]") + 1;
		char *toReturn = malloc(sizeof(char) * len);
		strcpy(toReturn, "[]");
		return toReturn;
	}
	else {
		iter = createIterator((List *)cal->events);
		while ((element = nextElement(&iter)) != NULL) {
			if (i == eventNo) {
				Event *event = (Event *)element;
				char *propJSON = propListToJSON(event->properties);
				deleteCalendar(cal);
				return propJSON;
			}
			else {
				i++;
			}
		}
	}
	return NULL;
}

char* createEvt(char* filename, char* uid, char* stampDate, char* stampTime, char* startTime, char* startDate,char* summary, int stampUTC, int startUTC) {
	ICalErrorCode err;
	Calendar* cal;
	char* toReturn;

	Event* event = JSONtoEvent(uid);
	if (event == NULL) {
		return NULL;
	}

	err = createCalendar(filename, &cal);
	addEventToCal(event, cal,stampDate,stampTime,startTime,startDate,summary,stampUTC,startUTC);
	err = writeCalendar(filename, cal);

	toReturn = printError(err);

	return toReturn;
}

void addEventToCal(Event* event, Calendar* cal, char* stampDate, char* stampTime, char* startTime, char* startDate,char* summary, int stampUTC, int startUTC) {

	strcpy(event->creationDateTime.date, stampDate);
	strcpy(event->creationDateTime.time, stampTime);
	if (stampUTC == 0) {
		event->creationDateTime.UTC = false;
	} else {
		event->creationDateTime.UTC = true;
	}
	strcpy(event->startDateTime.date, startDate);
	strcpy(event->startDateTime.time, startTime);
	if (startUTC == 0) {
		event->startDateTime.UTC = false;
	} else {
		event->startDateTime.UTC = true;
	}

	if (strcmp(summary, "") != 0) {
		Property* prop = initializeProperty(summary, "SUMMARY");
		insertBack(event->properties, (void*)prop);
	}

	addEvent(cal,event);
}

char* createNewCal(char* filename, char* calJSON, char* uid, char* stampDate, char* stampTime, char* startTime, char* startDate,char* summary, int stampUTC, int startUTC) {
	Calendar* cal;
	char* toReturn;

	Event* event = JSONtoEvent(uid);

	cal = JSONtoCalendar(calJSON);
	addEventToCal(event, cal, stampDate, stampTime, startTime, startDate, summary, stampUTC, startUTC);

	ICalErrorCode error = validateCalendar(cal);

	if (error == OK) {
		error = writeCalendar(filename, cal);
	}

	toReturn = printError(error);
	return toReturn;
}