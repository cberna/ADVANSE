#include <stdio.h>
#include <stdlib.h>
#include <mysql.h>
#include <string.h>
#include <time.h>

#if defined POSIX
	#define CLEARSCR system("clear")
#elif defined MSDOS || defined WIN32
	#define CLEARSCR system("cls")
#endif

typedef char string1000[1001];
typedef char string800[801];
typedef char string500[501];
typedef char string300[301];
typedef char string150[151];
typedef char string100[101];
typedef char string50 [51];
typedef char string40[41];
typedef char string30[31];
typedef char string20[21];
typedef char string3[4];

/**UTILITY FUNCTIONS**/
void utilEscapeStringValue (char** stringValue) {
	char* actualString = *stringValue;

	if (actualString == NULL) {
		return;
	}

	unsigned long length = strlen(actualString);
	unsigned long stringCtr;
	char* newString = NULL;

	for (stringCtr = 0; stringCtr < length; stringCtr++) {
		char curChar = *(actualString + stringCtr);
		if (curChar == '\'') {
			
			if (newString == NULL) {
				length ++;
				newString = malloc(sizeof(char)*length+1);
			}
			else {
				length ++;
				newString = realloc(newString, sizeof(char)*length+1);
			}

			if (newString == NULL) {
				fprintf(stderr, "%s\n", "Memory for allocation has run out");
			}
			else {
				strncpy(newString, actualString, stringCtr + 1);
				strcat(newString, "'");
			}

		}
	}
    
    if (newString != NULL) {
        strcat(newString, actualString + stringCtr);
        free(actualString);
        actualString = newString;
    }
}

void utilFormatTime (const time_t time, char** timeString) {
    *timeString = malloc(sizeof(char) * 20);
	strftime(*timeString, 20, "%D - %H:%M", localtime(&time));
}

/**DATABASE CONNECTION**/

void utilPrintError (MYSQL* conn, char* message) {
	fprintf(stderr, "%s\n", message);

	if (conn != NULL) {
		fprintf(stderr, "Error %u - %s\n", mysql_errno(conn), mysql_error(conn));
	}
}

MYSQL *dbConnectDatabase (char* server, char* username, char* password, char* dbName,
	unsigned int portNumber, char* socketName, unsigned int flags) {

	MYSQL *conn;

	conn = mysql_init(NULL);

	if (conn == NULL) {
      printf("Error %u: %s\n", mysql_errno(conn), mysql_error(conn));
      //exit(1);
  	}

  	if (mysql_real_connect(conn, server, username, password, dbName, portNumber, socketName, flags) == NULL) {
      printf("Error %u: %s\n", mysql_errno(conn), mysql_error(conn));
      //exit(1);
  	}

  	return conn;

}

void dbDisconnectDatabase (MYSQL* mySqlInstance) {
	mysql_close(mySqlInstance);
}

MYSQL* dbConnectDefaultDatabase (char* username, char* password) {
  MYSQL *conn;

  conn = dbConnectDatabase("localhost", username, 
          password, "advanse", 0, NULL, 0);

  return conn;
}

/**SECTION MODULE**/
typedef struct {
	int sectionId;
	char* sectionName;
	char* professorName;
} Section;

struct sectionTable {
	char* tableName;
	char* sectionId;
	char* sectionName;
	char* professorName;
};

const struct sectionTable SectionTable = {
	"section",
	"section_id",
	"section_name",
	"professor_name"
};

/**USER MODULE**/
typedef struct {
	int userId;
	char* userName;
	char* password;
	int sectionId;
} User;

struct userTable {
	char* tableName;
	char* userId;
	char* userName;
	char* password;
	char* sectionId;
};

const struct userTable UserTable = {
	"user",
	"user_id",
	"user_name",
	"password",
	"section_id"
};

/**PHASE RETRIEVAL**/

typedef struct {
	int phaseId;
	char* phaseName;
} Phase;

struct phaseTable {
	char* tableName;
	char* phaseId;
	char* phaseName;
};

const struct phaseTable PhaseTable = {
	"phase", "phase_id", "phase_name"
};

Phase priConvertRow (MYSQL_ROW row, unsigned int numberOfFields) {
	int fieldCtr;
	Phase phase;

	for (fieldCtr = 0; fieldCtr < numberOfFields; fieldCtr++) {
		char* rowData = row[fieldCtr];

		if (fieldCtr == 0) {
			//PHASE ID
			phase.phaseId = rowData != NULL ? atoi(rowData) : -1;
		}
		else if (fieldCtr == 1) {
			//PHASE NAME
            if (rowData != NULL) {
            	phase.phaseName = malloc(sizeof(char) * (strlen(rowData) + 1));
                strcpy(phase.phaseName, rowData);
            }
            else {
                phase.phaseName = NULL;
            }
		}
		else {
			utilPrintError(NULL, "Unexpected column.");
		}
	}

	return phase;
}

unsigned long priGetPhases (MYSQL* conn, Phase** phase) {
	MYSQL_RES* resultSet;
	char query[300];

	sprintf(query, "SELECT * FROM %s", PhaseTable.tableName);

	if (mysql_query(conn, query) != 0) {
		utilPrintError(NULL, "The query is invalid.");
	}
	else {
		resultSet = mysql_store_result(conn);
		if (resultSet == NULL) {
			utilPrintError(conn, "The result set could not be stored.");
		}
		else {
			const unsigned long numberOfRows = (unsigned long)mysql_num_rows(resultSet);
			Phase* results = malloc(sizeof(Phase) * numberOfRows);

			MYSQL_ROW row;
			unsigned int rowCtr = 0;
			unsigned int numOfFields = mysql_num_fields(resultSet);

			while ((row = mysql_fetch_row(resultSet)) != NULL) {
				*(results + rowCtr) = priConvertRow(row, numOfFields);
				rowCtr++;
			}

			mysql_free_result(resultSet);
			*phase = results;
			return numberOfRows;
		}
	}
	return 0;
}

/**PROJECT**/
typedef struct {
	int userProjectId;
	int userId;
	int pspId;
	int currentPhase;
	int done;
} Project;

struct projectTable {
	char* userProjectId;
	char* userId;
	char* pspId;
	char* currentPhase;
	char* done;
};

const struct projectTable ProjectTable = {
	"user_project_id",
	"user_id",
	"psp_id",
	"current_phase",
	"done"
};

/**TIME RECORDING LOG**/

typedef struct {
	int logId;
	int userProjectId;
	int phaseId;
	time_t startTime;
	time_t endTime;
	char* reason;
} Record;

struct recordTable {
	char* tableName;
	char* logId;
	char* userProjectId;
	char* phaseId;
	char* startTime;
	char* endTime;
	char* remarks;
};

const struct recordTable RecordTable = {
	"timelog", 
	"log_id", 
	"user_project_id", 
	"phase_id", 
	"start_time", 
	"end_time", 
	"remarks"
};

typedef struct {
	int interruptionId;
	int logId;
	time_t startTime;
	time_t endTime;
	char* reason;
} Interruption;

struct interruptionTable {
	char* tableName;
	char* interruptionId;
	char* logId;
	char* startTime;
	char* endTime;
	char* reason;
};

const struct interruptionTable InterruptionTable = {
	"interruption", 
	"interruption_id", 
	"log_id", 
	"start_time", 
	"end_time", 
	"reason"
};

Interruption trlConvertInterruptionRow(MYSQL_ROW row, unsigned int numberOfFields) {
	Interruption interruption;

	int fieldCtr;
	for (fieldCtr = 0; fieldCtr < numberOfFields; fieldCtr++) {
		char* rowData = row[fieldCtr];

		if (fieldCtr == 0) {
			//interruption id
			interruption.interruptionId = rowData != NULL ? atoi(rowData) : -1;
		}
		else if (fieldCtr == 1) {
			//log id
			interruption.logId = rowData != NULL ? atoi(rowData) : -1;
		}
		else if (fieldCtr == 2) {
			//start time
			interruption.startTime = rowData != NULL ? atol(rowData) : 0;
		}
		else if (fieldCtr == 3) {
			//end time
			interruption.endTime = rowData != NULL ? atol(rowData) : 0;
		}
		else if (fieldCtr == 4) {
			//reason
			if (rowData != NULL) {
				interruption.reason = malloc(sizeof(char) * (strlen(rowData) + 1));
				strcpy(interruption.reason, rowData);
			}
			else {
				interruption.reason = NULL;
			}
		}
		else {
			utilPrintError(NULL, "Unexpected column during conversion to interruption object.");
		}
	}

	return interruption;

}

void trlInsertRecord (MYSQL* conn, Record timeLog) {
	if (timeLog.userProjectId < 1) {
		utilPrintError(NULL, "The project specified is not valid.");
		return;
	}

	char insertQuery[300];

	if (timeLog.reason != NULL) {
		//escape the string in the reason field
		utilEscapeStringValue(&(timeLog.reason));
	
		sprintf(insertQuery, "INSERT INTO %s (%s, %s, %s, %s, %s) VALUES (%d, %d, %ld, %ld, '%s')",
			RecordTable.tableName, 
			RecordTable.userProjectId, RecordTable.phaseId, RecordTable.startTime, RecordTable.endTime, RecordTable.remarks,
			timeLog.userProjectId, timeLog.phaseId, timeLog.startTime, timeLog.endTime, timeLog.reason);
	}
	else {
		sprintf(insertQuery, "INSERT INTO %s (%s, %s, %s, %s) VALUES (%d, %d, %ld, %ld)",
			RecordTable.tableName, 
			RecordTable.userProjectId, RecordTable.phaseId, RecordTable.startTime, RecordTable.endTime,
			timeLog.userProjectId, timeLog.phaseId, timeLog.startTime, timeLog.endTime);
	}	

	if (mysql_query(conn, insertQuery) != 0) {
		utilPrintError(conn, "INSERT statement failed");
	}

}

void trlUpdateRecord (MYSQL* conn, Record record) {
	if (record.userProjectId < 1) {
		utilPrintError(NULL, "The project specified is not valid.");
		return;
	}

	char updateQuery[300];

	if (record.reason != NULL) {
		//escape the string in the reason field
		utilEscapeStringValue(&(record.reason));

		sprintf(updateQuery, "UPDATE %s SET %s = %ld, %s = %ld, %s = '%s' WHERE %s = %d",
			RecordTable.tableName, 
			RecordTable.startTime, record.startTime,
			RecordTable.endTime, record.endTime,
			RecordTable.remarks, record.reason,
			RecordTable.logId, record.logId);	
	}
	else {
		sprintf(updateQuery, "UPDATE %s SET %s = %ld, %s = %ld WHERE %s = %d",
			RecordTable.tableName, 
			RecordTable.startTime, record.startTime,
			RecordTable.endTime, record.endTime,
			RecordTable.logId, record.logId);	
	}
	

	if (mysql_query(conn, updateQuery) != 0) {
		utilPrintError(conn, "UPDATE statement failed");
	}
}

Record trlConvertRecordRow (MYSQL_ROW row, unsigned int numberOfFields) {
	int fieldCtr;
	Record record;

	for (fieldCtr = 0; fieldCtr < numberOfFields; fieldCtr++) {
		char* rowData = row[fieldCtr];

		if (fieldCtr == 0) {
			//log id
			record.logId = rowData != NULL ? atoi(rowData) : -1;
		}
		else if (fieldCtr == 1) {
			//user project id
			record.userProjectId = rowData != NULL ? atoi(rowData) : -1;
		}
		else if (fieldCtr == 2) {
			//phase id
			record.phaseId = rowData != NULL ? atoi(rowData) : -1;
		}
		else if (fieldCtr == 3) {
			//start time
			record.startTime = rowData != NULL ? atol(rowData) : 0;
		}
		else if (fieldCtr == 4) {
			//end time
			record.endTime = rowData != NULL ? atol(rowData) : 0;
		}
		else if (fieldCtr == 5) {
			//reason
			if (rowData != NULL) {
				record.reason = malloc(sizeof(char) * (strlen(rowData) + 1));
				strcpy(record.reason, rowData);
			}
			else {
				record.reason = NULL;
			}
		}
		else {
			utilPrintError(NULL, "Unexpected column.");
		}
	}

	return record;
}

unsigned long trlGetTimeLogsForProject (MYSQL* conn, Project project, Record** records) {
	MYSQL_RES* resultSet;
	char query[300];

	sprintf(query, "SELECT * FROM %s WHERE %s = %d",
		RecordTable.tableName,
		RecordTable.userProjectId, project.userProjectId);

	if (mysql_query(conn, query) != 0) {
		utilPrintError(NULL, "The query is invalid. Can't retrieve time logs for project.");
	}
	else {
		resultSet = mysql_store_result(conn);
		if (resultSet == NULL) {
			utilPrintError(conn, "The result set could not be stored.");
		}
		else {
			const unsigned long numberOfRows = (unsigned long)mysql_num_rows(resultSet);
			Record* results = malloc(sizeof(Record) * numberOfRows);

			MYSQL_ROW row;
			unsigned int rowCtr = 0;
			unsigned int numOfFields = mysql_num_fields(resultSet);

			while ((row = mysql_fetch_row(resultSet)) != NULL) {
				*(results + rowCtr) = trlConvertRecordRow(row, numOfFields);
			}

			mysql_free_result(resultSet);
			*records = results;
			return numberOfRows;
		}
	}

	return 0;
}

Record* trlGetTimeLogForPhase (MYSQL* conn, Project project, Phase phase) {
	MYSQL_RES* resultSet;
	char query[300];

	sprintf(query, "SELECT * FROM %s WHERE %s = %d AND %s = %d",
		RecordTable.tableName,
		RecordTable.userProjectId, project.userProjectId,
		RecordTable.phaseId, phase.phaseId);

	if (mysql_query(conn, query) != 0) {
		utilPrintError(NULL, "The query is invalid. Can't retrieve time log for specific phase.");
	}
	else {
        resultSet = mysql_store_result(conn);
        if (resultSet == NULL) {
            utilPrintError(conn, "Query failed. Couldn't retrieve time logs per phase");
        }
        else {
            const unsigned long numberOfRows = (unsigned long)mysql_num_rows(resultSet);
            Record* results = malloc(sizeof(Record) * numberOfRows);
            
            MYSQL_ROW row;
            unsigned int rowCtr = 0;
            unsigned int numOfFields = mysql_num_fields(resultSet);
            
            while ((row = mysql_fetch_row(resultSet)) != NULL) {
                *(results + rowCtr) = trlConvertRecordRow(row, numOfFields);
                rowCtr++;
            }
            
            mysql_free_result(resultSet);
            
            if (numberOfRows == 0) {
                return NULL;
            }
            else {
                return results;
            }
        }
	}

	return NULL;
}

void trlStartRecord (MYSQL* conn, Project project, Phase phase) {
	Record* newTimeLog = malloc(sizeof(Record));
	newTimeLog->userProjectId = project.userProjectId;
	newTimeLog->phaseId = phase.phaseId;
	newTimeLog->startTime = time(NULL);
	newTimeLog->endTime = 0;
	newTimeLog->reason = NULL;
	trlInsertRecord(conn, *newTimeLog);
	free(newTimeLog);
}

void trlStopRecord (MYSQL* conn, Project project, Phase phase) {
	Record* record = trlGetTimeLogForPhase(conn, project, phase);

	if (record == NULL) {
		utilPrintError(NULL, "The time recording log to be updated is not found.");
		return;
	}
	else {
		Record previousRecord = *record;
		previousRecord.endTime = time(NULL);
		trlUpdateRecord(conn, previousRecord);
		free(record);
	}
}

int trlIsPhaseFinished (MYSQL* conn, Project project, Phase phase) {
    Record* associatedRecord = trlGetTimeLogForPhase(conn, project, phase);
    
    if (associatedRecord != NULL) {
        if (associatedRecord->endTime == 0) {
            return 0;
        }
        else {
            return 1;
        }
    }
    
    return 0;
}

void trlInsertInterruption (MYSQL* conn, Interruption interruption) {
	if (interruption.logId < 1) {
		utilPrintError(NULL, "The time log specified is not valid.");
		return;
	}

	char insertQuery[300];

	if (interruption.reason != NULL) {
		//escape the string in the reason field
		utilEscapeStringValue(&(interruption.reason));
		sprintf(insertQuery, "INSERT INTO %s (%s, %s, %s, %s) VALUES (%d, %ld, %ld, '%s')",
			InterruptionTable.tableName,
			InterruptionTable.logId, InterruptionTable.startTime, InterruptionTable.endTime, InterruptionTable.reason,
			interruption.logId, interruption.startTime, interruption.endTime, interruption.reason);
	}
	else {
		sprintf(insertQuery, "INSERT INTO %s (%s, %s, %s) VALUES (%d, %ld, %ld)",
			InterruptionTable.tableName,
			InterruptionTable.logId, InterruptionTable.startTime, InterruptionTable.endTime,
			interruption.logId, interruption.startTime, interruption.endTime);
	}

	if (mysql_query(conn, insertQuery) != 0) {
		utilPrintError(NULL, "INSERT statement failed");
	}
}

void trlUpdateInterruption (MYSQL* conn, Interruption interruption) {
	if (interruption.logId < 1) {
		utilPrintError(NULL, "The time log specified is not valid.");
		return;
	}

	char updateQuery[300];

	if (interruption.reason != NULL) {
		//escape the string in the reason field
		utilEscapeStringValue(&(interruption.reason));
		sprintf(updateQuery, "UPDATE %s SET %s = %ld, %s = %ld, %s = '%s' WHERE %s = %d",
			InterruptionTable.tableName,
			InterruptionTable.startTime, interruption.startTime,
			InterruptionTable.endTime, interruption.endTime,
			InterruptionTable.reason, interruption.reason,
			InterruptionTable.interruptionId, interruption.interruptionId);
	}
	else {
		sprintf(updateQuery, "UPDATE %s SET %s = %ld, %s = %ld WHERE %s = %d",
			InterruptionTable.tableName,
			InterruptionTable.startTime, interruption.startTime,
			InterruptionTable.endTime, interruption.endTime,
			InterruptionTable.interruptionId, interruption.interruptionId);
	}

	if (mysql_query(conn, updateQuery) != 0) {
		utilPrintError(NULL, "UPDATE statement failed");
	}
}

void trlStartInterruption (MYSQL* conn, Project project, Phase phase) {
	Interruption* newInterruption = malloc(sizeof(Interruption));
	newInterruption->startTime = time(NULL);
	newInterruption->endTime = 0;
	newInterruption->reason = NULL;

	//retrieve trl entry for current phase and project
	Record* trlEntry = trlGetTimeLogForPhase(conn, project, phase);

	if (trlEntry == NULL) {
		utilPrintError(NULL, "The associated time log for this interruption could not be found.");
	}
	else {
		Record record = *trlEntry;
		newInterruption->logId = record.logId;

		trlInsertInterruption(conn, *newInterruption);
		free(trlEntry);
	}

	free(newInterruption);
}

Interruption* trlGetUnendedInterruptionInLog (MYSQL* conn, Record record) {
	MYSQL_RES* resultSet;
	char query[300];

	sprintf(query, "SELECT * FROM %s WHERE %s = %d AND %s = %d",
		InterruptionTable.tableName,
		InterruptionTable.logId, record.logId,
		InterruptionTable.endTime, 0);

	if (mysql_query(conn, query) != 0) {
		utilPrintError(NULL, "The query is invalid. Can't retrieve unended interruption");
	}
	else {
        resultSet = mysql_store_result(conn);
        
        if (resultSet == NULL) {
            utilPrintError(conn, "Query failed. Unable to get unended interruption based on record.");
        }
        else {
            const unsigned long numberOfRows = (unsigned long)mysql_num_rows(resultSet);
            Interruption* results = malloc(sizeof(Interruption) * numberOfRows);
            
            MYSQL_ROW row;
            unsigned int rowCtr = 0;
            unsigned int numOfFields = mysql_num_fields(resultSet);
            
            while ((row = mysql_fetch_row(resultSet)) != NULL) {
                *(results + rowCtr) = trlConvertInterruptionRow(row, numOfFields);
                rowCtr++;
            }
            
            mysql_free_result(resultSet);
            
            if (numberOfRows == 0) {
                return NULL;
            }
            else {
                return results;			
            }
        }
	}

	return NULL;
}

Interruption* trlGetUnendedInterruption (MYSQL* conn, Project project, Phase phase) {
	Record* phaseLog = trlGetTimeLogForPhase(conn, project, phase);

	if (phaseLog == NULL) {
		return NULL;
	}
	else {
		Interruption* unendedInterruption = trlGetUnendedInterruptionInLog(conn, *phaseLog);
		free(phaseLog);
		return unendedInterruption;
	}
}

int trlIsPhaseInterrupted (MYSQL* conn, Project project, Phase phase) {
	Interruption* interruption = trlGetUnendedInterruption(conn, project, phase);
	if (interruption == NULL)
		return 0; //false
	else {
		free(interruption);
		return 1; //true
	}
}

void trlStopInterruption (MYSQL* conn, Project project, Phase phase, char* reason) {
	Interruption* lastInterruption = trlGetUnendedInterruption(conn, project, phase);

	if (lastInterruption == NULL) {
		utilPrintError(NULL, "The associated interruption to be updated with a stop time cannot be found.");
		return;
	}
	else {
		Interruption updatedInterruption = *lastInterruption;
		updatedInterruption.endTime = time(NULL);
		updatedInterruption.reason = reason;		
		trlUpdateInterruption(conn, updatedInterruption);
		free(lastInterruption);
	}
}

double trlGetInterruptionTime (Interruption* interruptions, long numOfInterruptions) {
	double interruptionTotal = 0;

	if (interruptions != NULL) {
		int interruptCtr;
		for (interruptCtr = 0; interruptCtr < numOfInterruptions; interruptCtr++) {
			Interruption currentInterruption = *(interruptions + interruptCtr);
			
			if (currentInterruption.startTime > 0 && currentInterruption.endTime > 0) {
				interruptionTotal += difftime(currentInterruption.endTime, currentInterruption.startTime);
			}
		}		
	}

	return interruptionTotal;
}

double trlGetDeltaTime (Record trlLog, Interruption* interruptions, long numOfInterruptions, double* interruptionTime) {
	if (trlLog.startTime == 0 || trlLog.endTime == 0) {
		//cannot compute yet if a log for a phase hasn't started nor ended yet
		return 0;
	}

	double interruptionTotal = 0;

	//We count the delta time for the interruptions first.
	interruptionTotal = trlGetInterruptionTime(interruptions, numOfInterruptions);	
	
	//count the time log's delta time
	double logDelta = difftime(trlLog.endTime, trlLog.startTime);
	logDelta -= interruptionTotal;

	if (interruptionTime != NULL) {
		*(interruptionTime) = interruptionTotal;
	}

	return logDelta;
}

unsigned long trlGetInterruptionsForTimeLog (MYSQL* conn, Record* record, Interruption** interruptions) {
	if (record == NULL) {
		utilPrintError(NULL, "There is no associated record with this phase for this project");	
	}
	else {
		MYSQL_RES* resultSet;

		int logId = record->logId;

		char searchQuery[300];
		sprintf(searchQuery, "SELECT * FROM %s WHERE %s = %d",
			InterruptionTable.tableName,
			InterruptionTable.logId, logId
		);

		if (mysql_query(conn, searchQuery) != 0) {
			utilPrintError(NULL, "The query is invalid. Can't retrieve interruptions");
		}
		else {
			resultSet = mysql_store_result(conn);
			if (resultSet == NULL) {
				utilPrintError(conn, "The result set could not be stored");
			}
			else {
				const unsigned long numberOfRows = (unsigned long)mysql_num_rows(resultSet);
				Interruption* results = malloc(sizeof(Interruption) * numberOfRows);

				MYSQL_ROW row;
				unsigned int rowCtr = 0;
				unsigned int numOfFields = mysql_num_fields(resultSet);

				while ((row = mysql_fetch_row(resultSet)) != NULL) {
					*(results + rowCtr) = trlConvertInterruptionRow(row, numOfFields);
					rowCtr++;
				}

				mysql_free_result (resultSet);
				*interruptions = results;

				return numberOfRows;
			}
		}
	}

	return 0;
}

unsigned long trlGetInterruptionsForPhase (MYSQL* conn, Project project, Phase phase, Interruption** interruptions) {
	//retrieve the appropriate time recording log given the project and the phase
	Record* record = trlGetTimeLogForPhase(conn, project, phase);
	unsigned long numberOfRecords = trlGetInterruptionsForTimeLog(conn, record, interruptions);
	free(record);
	return numberOfRecords;
}

void trlDisplayTimeRecordingLog (MYSQL* conn, Project project) {
	printf("Time Recording Log\n\n");

	//TODO : retrieve user using user id

	//display program name
	printf("Project Name: PSP %d\n\n", project.pspId);

	//Lay the table out
	printf("Start (Date - Time)\t\tStop (Date - Time)\t\tInterruption Time\t\tDelta Time\t\tPhase\t\tComments\n");

	Phase* phases = NULL;
	unsigned long numOfPhases = priGetPhases(conn, &phases);
	int phaseCtr;

	if (phases != NULL) {
		for (phaseCtr = 0; phaseCtr < numOfPhases; phaseCtr++) {
			Phase currentPhase = *(phases + phaseCtr);

			//retrieve trl entry per phase
			Record* record = trlGetTimeLogForPhase(conn, project, currentPhase);
			if (record == NULL) {
				//end of the time log so far
				break;
			}
			else {
				//start time
				char* startTimeString = NULL;
				utilFormatTime(record->startTime, &startTimeString);
				printf("%s\t\t", startTimeString);
				//end time
				if (record->endTime == 0) {
                    printf("N/A\t\t");
                }
                else {
                    char* endTimeString = NULL;
                    utilFormatTime(record->endTime, &endTimeString);
                    printf("%s\t\t", endTimeString);
                }

				//get interruptions
				Interruption* interruptions = NULL;
				unsigned long numOfInterruptions = trlGetInterruptionsForTimeLog(conn, record, &interruptions);

				//get delta time
				double interruptionTime = 0;
				double deltaTime = trlGetDeltaTime(*record, interruptions, numOfInterruptions, &interruptionTime);

				//interruption time
				printf("%g minutes\t\t", interruptionTime / 60);

				//delta time
				printf("%g minutes\t\t", deltaTime / 60);

				//phase
				printf("%s\t\t", currentPhase.phaseName);

				//comments
				if (interruptions != NULL) {
					int interruptionCtr;
					for (interruptionCtr = 0; interruptionCtr < numOfInterruptions; interruptionCtr++) {
						Interruption currentInterruption = *(interruptions + interruptionCtr);
						if (currentInterruption.reason != NULL) {
							printf("%s", currentInterruption.reason);
						}
						else {
							printf("No reason stated");
						}

						printf(",");
					}

					free(interruptions);
				}

				free(record);
			}

			printf("\n\n");

		}
		free(phases);
	}
	else {
		utilPrintError(NULL, "Something may be wrong with the database.");
	}

	printf("Nothing follows.\n\n");
}

/**UI INTEGRATION**/

int imSelectFromChoices(char* choices[], int numOfChoices, char* optionalPrompt) {
	int choice;
	int choiceCtr;
	int showErrorMessage = 0;

	for (choiceCtr = 0; choiceCtr < numOfChoices; choiceCtr++) {
		printf("%d - %s\n", choiceCtr + 1, choices[choiceCtr]);
	}
	printf("\n");

	do {
		if (showErrorMessage == 1) {
			printf("The input that was received was invalid. Please try again.\n");
		}

		if (optionalPrompt == NULL) {
			printf("Enter the correct number: ");
		}	
		else {
			printf("%s: ", optionalPrompt);
		}
		scanf("%d", &choice);
		showErrorMessage = 1;
	} while (choice <= 0 || choice > numOfChoices);

	return choice;
}

int imSelectYesOrNo(char* optionalPrompt) {
	char* choices[] = {"Yes", "No"};
	return imSelectFromChoices(choices, 2, optionalPrompt);
}

Phase* imPhaseSelect (MYSQL* conn) {
	
	printf("Select a phase.\n\n");
	
	Phase* phases = NULL;
	unsigned long numberOfPhases = priGetPhases(conn, &phases);
	int phaseCtr;

	for (phaseCtr = 0; phaseCtr < numberOfPhases; phaseCtr++) {
		Phase currentPhase = *(phases + phaseCtr);
		printf("%d - %s\n", phaseCtr + 1, currentPhase.phaseName);
	}

	printf("%d - Exit\n", phaseCtr + 1);

	printf("\n");
	int phaseChoice = 0;
	int error = 0;

	do {
		if (error == 1) {
			printf("Invalid input. Please within the range of 1 to %ld.\n", numberOfPhases);
		}
		printf("Type phase number: ");
		scanf("%d", &phaseChoice);	
		error = 1;	
	} while (phaseChoice > numberOfPhases + 1 || phaseChoice <= 0);

	if (phaseChoice == numberOfPhases + 1) {
		//exit
		return NULL;
	}
	else {
		Phase* phaseToReturn = malloc(sizeof(Phase));
		*phaseToReturn = *(phases + (phaseChoice - 1));
		free(phases);

		return phaseToReturn;
	}
}

int imShowInterruptOptions (MYSQL* conn, Project project, Phase phase) {
	//display option to stop interrupting
	char* choices[] = {"Continue logging", "Back"};

	int choice = imSelectFromChoices(choices, 2, "Select from the choices");

	if (choice == 1) {
		//would you like to state a reason for the interruption?
		int reasonChoice = imSelectYesOrNo("Would you like to state a reason for the interruption?");
		char* reason = NULL;

		if (reasonChoice == 1) {
            reason = malloc(sizeof(char) * 1000);
			printf("State a reason for the interruption (999 chars. max): ");
			scanf("%c", reason);
            fgets(reason, 1000, stdin);
            char* newLineChar = strchr(reason, '\n');
            *newLineChar = '\0';
//            printf("Reason: %s\n", reason);
		}

		trlStopInterruption(conn, project, phase, reason);
		return 1;
	}
	else {
		return 0;
	}

}

int imStartLoggingOptions (MYSQL* conn, Project project, Phase phase) {
	//display options to start logging
	char* choices[] = {"Start logging", "Back"};

	int choice = imSelectFromChoices(choices, 2, "Select from the choices");

	if (choice == 1) {
		//start logging
		trlStartRecord(conn, project, phase);
		return 1;
	}
	else {
		return 0;
	}
}

int imStopLoggingOptions (MYSQL* conn, Project project, Phase phase) {
    if (trlIsPhaseFinished(conn, project, phase)) {
        printf("This phase is finished. Press any key to continue.");
        
        getchar();
        getchar();
        
        return 0;
    }
    
	//display options to stop logging (work ongoing)
	char* choices[] = {"Stop logging", "Interrupt phase", "Back"};

	int choice = imSelectFromChoices(choices, 3, "Select from the choices");

	if (choice == 1) {
		//stop logging
		trlStopRecord(conn, project, phase);
		return 0;
	}
	else if (choice == 2) {
		//interrupt
		trlStartInterruption(conn, project, phase);
		return 1;
	}
	else {
		return 0;
	}
}

void imRecordTime (MYSQL* conn, Project project, Phase phase) {
	int interrupted = 0;
	int refresh = 1;

	while (refresh == 1) {
        //check if the phase being logged is interrupted
        interrupted = trlIsPhaseInterrupted(conn, project, phase);
		
		printf("LOGGING OPTIONS FOR %s\n\n", phase.phaseName);

		if (interrupted) {
			refresh = imShowInterruptOptions(conn, project, phase);
		}
		else {
			if (trlGetTimeLogForPhase(conn, project, phase) == NULL) {
				refresh = imStartLoggingOptions(conn, project, phase);
			}
			else {
				refresh = imStopLoggingOptions(conn, project, phase);
			}
		}
	}
}

int testMain () {
	MYSQL* conn = dbConnectDefaultDatabase ("root", "p@ssword");

	Project project;
	project.userProjectId = 1;
	project.userId = 1;
	project.pspId = 1;
	project.currentPhase = 1;
	project.done = 0;

	Phase* selectedPhase = NULL;

	do {
		selectedPhase = imPhaseSelect(conn);

		//ask to start time, stop time, interrupt
        if (selectedPhase != NULL) {
            imRecordTime(conn, project, *selectedPhase);
        }
	} while (selectedPhase != NULL);

	trlDisplayTimeRecordingLog(conn, project);

	dbDisconnectDatabase(conn);

	fflush(stdin);
	getchar();

	return 0;
}

/**TEST REPORT TEMPLATE**/

typedef struct{
	int testNumber;
	int userProjectId;
	string500 testObjective;
	string500 testConditions;
	string500 expectedResults;
	string500 actualResults;
	string500 testDescription;
}TestReport;

struct trtTable {
	string20 tableName;
	string20 testNumber;
	string20 userProjectId;
	string20 testObjective;
	string20 testConditions;
	string20 expectedResults;
	string20 actualResults;
	string20 testDescription;
};

const struct trtTable TrtTable = {
	"testreport",
	"test_number",
	"user_project_id",
	"test_objective",
	"test_conditions",
	"expected_results",
	"actual_results",
	"test_description"
};

int trtCountDetails(MYSQL* conn, string300 query){
	MYSQL_RES* result;
	MYSQL_ROW row;
	int count = 0;
	string300 countQuery;
	
	sprintf(countQuery, "%s", query);
	if(mysql_query(conn, countQuery) != 0){
		printf("Error: Count Query Failed.");
	}
	else{
		result = mysql_store_result(conn);
		if(result == NULL){
			utilPrintError(conn, "No count produced.");
		}
		else{
			while((row = mysql_fetch_row(result))){
				count = atoi(row[0]);
			}
		}
	}
	return count;
}

void trtConcatString(string300 query, int detail, int userProjectId, int testNum){
	string100 query1;
	string100 query2;
	string100 query3;
	char projId[11];
	char testNo[11];
	
	sprintf(projId, "%d", userProjectId);
	sprintf(testNo, "%d", testNum);
	
	strcpy(query2, "' AND test_number = '");
	strcpy(query3, "';");
	
	switch(detail){
		case 1:
			strcpy(query1, "SELECT test_objective FROM testreport WHERE user_project_id='");
			break;
		case 2:
			strcpy(query1, "SELECT test_description FROM testreport WHERE user_project_id='");
			break;
		case 3:
			strcpy(query1, "SELECT expected_results FROM testreport WHERE user_project_id='");
			break;
		case 4:
			strcpy(query1, "SELECT actual_results FROM testreport WHERE user_project_id='");
			break;
		case 5:
			strcpy(query1, "SELECT * FROM testreport WHERE user_project_id='");
			break;
		case 6:
			strcpy(query1, "SELECT COUNT(*) FROM testreport WHERE user_project_id='");
			break;
		case 7:
			strcpy(query1, "SELECT COUNT(*) FROM testreport WHERE actual_results IS NULL AND user_project_id='");
			break;
	}
	
	strcat(query1, projId);
	if(detail != 5 && detail != 6){	
		strcat(query1, query2);
		strcat(query1, testNo);	
	}
	strcat(query1, query3);
	strcpy(query, query1);
}

void trtSelectDetails(MYSQL* conn, string300 query){
	MYSQL_RES* result;
	MYSQL_ROW row;
	int num_fields;
	int i;
	string300 selectQuery;
	
	sprintf(selectQuery, "%s", query);
	if(mysql_query(conn, selectQuery) != 0){
		printf("Query failed \n");
	}
	else {
		result = mysql_store_result(conn);
		if(result == NULL){
			utilPrintError(conn, "No matching results found.");
		}
		else {
			num_fields = mysql_num_fields(result);
			while((row = mysql_fetch_row(result))){
				for(i = 0; i < num_fields; i++){
					printf("%s ", row[i] ? row[i] : "NULL");
				}
				printf("\n");
			}
		}
	}
}

void trtDisplayReport(MYSQL* conn, int userProjectId, int testNum){
	string300 query;
	
	printf("Test Number: %d\n", testNum);
	printf("Test Objective: ");
	trtConcatString(query, 1, userProjectId, testNum);
	trtSelectDetails(conn, query);
	printf("Test Description: ");
	trtConcatString(query, 2, userProjectId, testNum);
	trtSelectDetails(conn, query);
	printf("Expected Results: ");
	trtConcatString(query, 3, userProjectId, testNum);
	trtSelectDetails(conn, query);
	printf("Actual Results: ");
	trtConcatString(query, 4, userProjectId, testNum);
	trtSelectDetails(conn, query);
}

void trtDisplayAllReports(MYSQL* conn, int userProjectId){
	string300 queryCount;
	trtConcatString(queryCount, 6, userProjectId, 0);
	int entryCount = trtCountDetails(conn, queryCount);
	
	CLEARSCR;
	printf("Test Reports\n\n");
	int i;
	for(i = 1; i < entryCount + 1; i++){
		trtDisplayReport(conn, userProjectId, i);
		printf("\n");
	}
	getch();
}

int trtUpdateDetails(MYSQL* conn, string500 actualResults, int testNum){
	string300 updateQuery;
	
	sprintf(updateQuery, "UPDATE %s SET %s='%s' WHERE %s = %d",
		TrtTable.tableName,
		TrtTable.actualResults, actualResults,
		TrtTable.testNumber, testNum);
	if(mysql_query(conn, updateQuery) != 0){
		return 0;
	}
	else {
		return 1;
	}
}

void trtInputActualResults(MYSQL* conn, int userProjectId){
	string300 querySelect;
	strcpy(querySelect, "SELECT test_number, test_description FROM testreport WHERE actual_results IS NULL;");
	int choice;
	int trans;
	string500 actualResults;
	int entryCount;
	string300 queryCount;
	char ans, dump;
	
	do{
		CLEARSCR;
		printf("Add Actual Test Results\n\n");
		trtSelectDetails(conn, querySelect);
			
		printf("\n\nChoose a test number: ");
		scanf("%d%c", &choice, &dump);
		
		trtConcatString(queryCount, 7, userProjectId, choice);
		entryCount = trtCountDetails(conn, queryCount);
		
		if(entryCount == 0){
			printf("Invalid input, choose from test numbers listed above...");
			getch();
		}
	} while(entryCount == 0);
	
	CLEARSCR;
	trtDisplayReport(conn, userProjectId, choice);
	printf("\n\nActual Results: ");
	gets(actualResults);
	
	printf("Save the details (Y/N)?");
	scanf("%c%c", &ans, &dump);
	if(ans == 'y' || ans == 'Y'){
		trans = trtUpdateDetails(conn, actualResults, choice);
		if(!trans){
			printf("\nError: Transaction Failed; Check database status...\n");
		}
		else{
			printf("\nActual Results Updated!");
		}
		getch();
	}
}

int trtInsertDetails(MYSQL* conn, TestReport testReport){
	string1000 insertQuery;
	
	sprintf(insertQuery, "INSERT INTO %s(%s, %s, %s, %s, %s, %s) VALUES (%d, (SELECT %s FROM project WHERE %s = %d), '%s', '%s', '%s', '%s');",
		TrtTable.tableName,
		TrtTable.testNumber, TrtTable.userProjectId, TrtTable.testObjective,
		TrtTable.testConditions, TrtTable.expectedResults, TrtTable.testDescription,
		testReport.testNumber, TrtTable.userProjectId, TrtTable.userProjectId,
		testReport.userProjectId, testReport.testObjective, testReport.testConditions,
		testReport.expectedResults, testReport.testDescription);
	
	if(mysql_query(conn, insertQuery) != 0){
		return 0;
	}
	else {
		return 1;
	}
}

void trtGetInput(MYSQL* conn, int userProjectId){
	TestReport testReport;
	string300 queryCount;
	trtConcatString(queryCount, 6, userProjectId, 0);
	int entryCount = trtCountDetails(conn, queryCount);
	int testNum = 1 + entryCount;
	char ans, dump;
	
	CLEARSCR;
	printf("Add Test Report\n\n");
	testReport.testNumber = testNum;
	printf("Test Number: %d\n", testReport.testNumber);
	testReport.userProjectId = userProjectId;
	printf("Test Objective: ");
	gets(testReport.testObjective);
	printf("Test Description: ");
	gets(testReport.testDescription);
	printf("Test Conditions: ");
	gets(testReport.testConditions);
	printf("Expected Results: ");
	gets(testReport.expectedResults);
	
	printf("Save the details (Y/N)?");
	scanf("%c", &ans);
	if(ans == 'y' || ans == 'Y'){
		int trans = trtInsertDetails(conn, testReport);
		if(!trans){
			printf("\nError: Transaction Failed; Check database status...\n");
		}
		else{
			printf("\nReport Added!");
		}
		getch();
	}
	else{
	}
}

void trtMenu(MYSQL* conn, int userProjectId){
	int choice;
	char dump;
	
	do{
		CLEARSCR;
		printf("Test Report Template\n\n");
		printf("[1] Add Test Report\n");
		printf("[2] Add Actual Results\n");
		printf("[3] View Test Reports\n\n");
		printf("[0] Go Back\n\n");
		printf("Choice: ");
		scanf("%d%c", &choice, &dump);
		
		switch(choice){
			case 1: trtGetInput(conn, userProjectId); break;
			case 2: trtInputActualResults(conn, userProjectId); break;
			case 3: trtDisplayAllReports(conn, userProjectId); break;
			case 0: /*BACK to*/break;
			default: printf("Invalid Input - choices are 1,2,3, or 4 only...");break;
		}
	} while(choice != 4);
}

/**PROCESS IMPROVEMENT PLAN**/

typedef struct{
	int piplanId;
	int userProjectId;
	string800 notes;
}ProcImpPlan;

struct procImpPlanTable{
	string20 tableName;
	string20 piplanId;
	string20 userProjectId;
	string20 notes;
};

const struct procImpPlanTable ProcImpPlanTable = {
	"piplan",
	"piplan_id",
	"user_project_id",
	"notes"
};

typedef struct{
	int problemId;
	int piplanId;
	string300 description;
}ProcImpProb;

struct procImpProbTable{
	string20 tableName;
	string20 problemId;
	string20 piplanId;
	string20 description;
};

const struct procImpProbTable ProcImpProbTable = {
	"pipproblem",
	"problem_id",
	"piplan_id",
	"description"
};

typedef struct{
	int proposalId;
	int piplanId;
	string300 description;
}ProcImpProp;

struct procImpPropTable{
	string20 tableName;
	string20 proposalId;
	string20 piplanId;
	string20 description;
};

const struct procImpPropTable ProcImpPropTable = {
	"pipproposal",
	"proposal_id",
	"piplan_id",
	"description"
};

int pipCountDetails(MYSQL* conn, string300 query){
	MYSQL_RES* result;
	MYSQL_ROW row;
	int count = 0;
	string300 countQuery;
	
	sprintf(countQuery, "%s", query);
	if(mysql_query(conn, countQuery) != 0){
		printf("Error: Count Query Failed.");
	}
	else{
		result = mysql_store_result(conn);
		if(result == NULL){
			utilPrintError(conn, "No count produced.");
		}
		else{
			while((row = mysql_fetch_row(result))){
				count = atoi(row[0]);
			}
		}
	}
	
	return count;
}

int pipUpdateDetails(MYSQL* conn, string800 notes, int piplanId){
	string1000 updateQuery;
	
	sprintf(updateQuery, "UPDATE %s SET %s = '%s' WHERE %s = %d;",
		ProcImpPlanTable.tableName,
		ProcImpPlanTable.notes, notes,
		ProcImpPlanTable.piplanId, piplanId);
	if(mysql_query(conn, updateQuery) != 0){
		return 0;
	}
	else {
		return 1;
	}
}

void pipSelectDetails(MYSQL* conn, int details, int piplanId){
	string300 query;
	switch(details){
		case 1: strcpy(query, "SELECT problem_id, description FROM pipproblem WHERE piplan_id = "); 
			break;
		case 2: strcpy(query, "SELECT proposal_id, description FROM pipproposal WHERE piplan_id = "); 
			break;
		case 3: strcpy(query, "SELECT notes FROM piplan WHERE piplan_id = ");
			break;
	}
	char pipId[11];
	sprintf(pipId, "%d", piplanId);
	strcat(query, pipId);
	strcat(query, ";");
	
	MYSQL_RES* result;
	MYSQL_ROW row;
	int num_fields;
	int i;
	string300 selectQuery;
	
	sprintf(selectQuery, "%s", query);
	if(mysql_query(conn, selectQuery) != 0){
		printf("Query failed \n");
	}
	else {
		result = mysql_store_result(conn);
		if(result == NULL){
			utilPrintError(conn, "No matching results found.");
		}
		else {
			num_fields = mysql_num_fields(result);
			while((row = mysql_fetch_row(result))){
				for(i = 0; i < num_fields; i++){
					printf("%s           ", row[i] ? row[i] : "NULL");
				}
				printf("\n");
			}
		}
	}
}

int pipInsertDetails(MYSQL* conn, int detail, int id, int piplanId, 
	string300 description){
	string1000 query;
	switch(detail){
		case 1: sprintf(query, "INSERT INTO %s VALUES (%d, (SELECT %s FROM %s WHERE %s = %d), '%s');", 
			ProcImpProbTable.tableName, 
			id, ProcImpProbTable.piplanId, ProcImpPlanTable.tableName, 
			ProcImpProbTable.piplanId, piplanId, description); 
			break;
		case 2: sprintf(query, "INSERT INTO %s VALUES (%d, (SELECT %s FROM %s WHERE %s = %d), '%s');", 
			ProcImpPropTable.tableName, 
			id, ProcImpPropTable.piplanId, ProcImpPlanTable.tableName, 
			ProcImpPropTable.piplanId, piplanId, description); 
			break;
		case 3: sprintf(query, "INSERT INTO %s(%s, %s) VALUES (%d, (SELECT %s FROM project WHERE %s = %d));", 
			ProcImpPlanTable.tableName, 
			ProcImpPlanTable.piplanId, ProcImpPlanTable.userProjectId, piplanId, 
			ProcImpPlanTable.userProjectId, ProcImpPlanTable.userProjectId, id); 
			break;
	}
	if(mysql_query(conn, query) != 0){
		return 0;
	}
	else {
		return 1;
	}
}

void pipDisplayPlan(MYSQL* conn, int piplanId){
	CLEARSCR;
	printf("Process Improvement Plan\n\n");
	printf("PIP Number  Problem Description\n");
	pipSelectDetails(conn, 1, piplanId);
	printf("\nProposal    Problem Description\nPIP Number\n");
	pipSelectDetails(conn, 2, piplanId);
	printf("\n\nNotes and Comments\n");
	pipSelectDetails(conn, 3, piplanId);
}

void pipInputNotes(MYSQL* conn, ProcImpPlan procImpPlan){
	char ans;
	char dump;
	printf("Done adding problem and proposal?\n");
	printf("Do you wish to add notes and comments now?");
	scanf("%c%c", &ans, &dump);
	int trans;
	
	if(ans == 'y' || ans == 'Y'){
		pipDisplayPlan(conn, procImpPlan.piplanId);
		printf("\n\nNotes and Comments:\n");
		gets(procImpPlan.notes);
		printf("\n\nSave the details (Y/N)? ");
		scanf("%c", &ans);
		
		if(ans == 'y' || ans == 'Y'){
			trans = pipUpdateDetails(conn, procImpPlan.notes, 
						procImpPlan.piplanId);
			if(!trans){
				printf("\nError: Transaction Failed; Check database status...\n");
			}
			else{
				printf("Notes and Comments Added!");
			}
			getch();
		}
	}
}

pipInputProposal(MYSQL* conn, int piplanId){
	char ans;
	ProcImpProp procImpProp;
	procImpProp.piplanId = piplanId;
	pipDisplayPlan(conn, piplanId);
	string300 query;
	strcpy(query, "SELECT COUNT(*) FROM pipproposal WHERE piplan_id = ");
	char projId[11];
	sprintf(projId, "%d", piplanId);
	strcat(query, projId);
	strcat(query, ";");
	procImpProp.proposalId = 1 + pipCountDetails(conn, query);
	printf("\n\nProposal ID: %d\n", procImpProp.proposalId);
	printf("Description\n");
	gets(procImpProp.description);
	printf("\n\nSave the details (Y/N)? ");
	scanf("%c", &ans);
	
	if(ans == 'y' || ans == 'Y'){
		int trans = pipInsertDetails(conn, 2, procImpProp.proposalId, 
					procImpProp.piplanId, procImpProp.description);
		if(!trans){
			printf("\nError: Transaction Failed; Check database status...");
		}
		else{
			printf("Proposal Added!");
		}
		getch();
	}
}

void pipInputProblem(MYSQL* conn, int piplanId){
	char ans;
	ProcImpProb procImpProb;
	procImpProb.piplanId = piplanId;
	pipDisplayPlan(conn, piplanId);
	string300 query;
	strcpy(query, "SELECT COUNT(*) FROM pipproblem WHERE piplan_id=");
	char projId[11];
	sprintf(projId, "%d", piplanId);
	strcat(query, projId);
	strcat(query, ";");
	procImpProb.problemId = 1 + pipCountDetails(conn, query);
	printf("\n\nProblem ID: %d\n", procImpProb.problemId);
	printf("Description\n");
	gets(procImpProb.description);
	printf("\n\nSave details (Y/N)? ");
	scanf("%c", &ans);
	
	if(ans == 'y' || ans == 'Y'){
		int trans = pipInsertDetails(conn, 1, procImpProb.problemId, 
					procImpProb.piplanId, procImpProb.description);
		if(!trans){
			printf("\nError: Transaction Failed; Check database status...\n");
		}
		else{
			printf("Problem Added!");
		}
		getch();
	}
}

void pipMenu(MYSQL* conn, int userProjectId){
	int choice;
	char dump;
	ProcImpPlan procImpPlan;
	procImpPlan.userProjectId = userProjectId;
	string300 queryCount;
	strcpy(queryCount, "SELECT COUNT(*) FROM piplan WHERE user_project_id =");
	char projId[11];
	sprintf(projId, "%d", userProjectId);
	strcat(queryCount, projId);
	strcat(queryCount, ";");
	int idExist = pipCountDetails(conn, queryCount);
	if(idExist == 0){
		string300 queryCountPipId;
		strcpy(queryCountPipId, "SELECT COUNT(*) FROM piplan;");
		procImpPlan.piplanId = 1 + pipCountDetails(conn, queryCountPipId);
		string800 notes;
		strcpy(notes, "Place Notes and Comments here...");
		pipInsertDetails(conn, 3, procImpPlan.userProjectId, 
			procImpPlan.piplanId, notes);
	}
	else{
		procImpPlan.piplanId = idExist;
	}
	do{
		CLEARSCR;
		printf("Process Improvement Plan\n\n");
		printf("[1] Add Problem\n");
		printf("[2] Add Proposal \n");
		printf("[3] Add Notes \n");
		printf("[4] View Process Improvement Plan \n\n");
		printf("[0] Go Back \n\n");
		printf("Choice: ");
		scanf("%d%c", &choice, &dump);
		
		switch(choice){
			case 1: pipInputProblem(conn, procImpPlan.piplanId); break;
			case 2: pipInputProposal(conn, procImpPlan.piplanId); break;
			case 3: pipInputNotes(conn, procImpPlan); break;
			case 4: pipDisplayPlan(conn, procImpPlan.piplanId); getch(); break;
			case 0: /*BACK to*/break;
			default: printf("Invalid Input, choices are 1, 2, 3, 4, or 5 only...");
		}
	}while(choice != 5);
}

/**USER SECTION MODULE**/

struct usmUserAccountStruct
{
  string40 usmUserName;
  string20 usmPassword;
  string3 usmSection;
};

typedef struct usmUserAccountStruct usmUserAccount;

void usmLogin(MYSQL* conn);
void usmSignUp(MYSQL* conn);
void usmInputPassword(string20 usmPassword);
void usmInsertUser(MYSQL* conn, usmUserAccount usmAccount);
int usmDataExists(MYSQL* conn, char * basis, char * basis_column, char * basis_table);
int usmVerifyAccount(MYSQL* conn, usmUserAccount usmLoginDetails);
void usmChooseSections(MYSQL* conn, string3 usmSectionTemp);
void pmGenerateProjects(MYSQL *conn, string40 userName);
void pmViewProjects(MYSQL *conn, string40 usmUserName);
void pmSetProjectDone(MYSQL* conn, string40 userName, int pspNum); //TO USE
void uiSectionManagement(MYSQL* conn); 

void usmLogin(MYSQL* conn){
  char usmInput = ' ';
  int usmLength = 0;
  usmUserAccount usmLoginDetails;
  system("cls");
  printf("Log In\n\n");
  printf("User name: ");
  scanf("%s", &usmLoginDetails.usmUserName);
  printf("Password: ");
  usmInputPassword(usmLoginDetails.usmPassword);
  printf("\n%s,%s\n", usmLoginDetails.usmUserName, usmLoginDetails.usmPassword);
  if(usmVerifyAccount(conn, usmLoginDetails)){
     printf("\nAccess Granted");
     pmViewProjects(conn, usmLoginDetails.usmUserName);
  }else{
   printf("Access Error");
   getch();
   uiDisplayMain();
  }
  getch();
}

void usmSignup(MYSQL* conn){
  char usmInput = ' ';
  int usmLength = 0;
  string20 usmCreatedPassword;
  string20 usmConfirmedPassword;
  usmUserAccount usmLoginDetails;
  system("cls");
  printf("Sign Up");
  printf("\nUser name: ");
  scanf("%s", &usmLoginDetails.usmUserName);
  do{
    printf("\nCreate Password: ");
    usmInputPassword(usmCreatedPassword);
    printf("\nConfirm Password: ");
    usmInputPassword(usmConfirmedPassword);
    if(strcmp(usmCreatedPassword, usmConfirmedPassword))
      printf("\nPassword do not match");
  }while(strcmp(usmCreatedPassword, usmConfirmedPassword));
  strcpy(usmLoginDetails.usmPassword, usmCreatedPassword);
  string3 usmSectionTemp;
  
  printf("\n\nSection: ");
  usmChooseSections(conn, usmSectionTemp);
  strcpy(usmLoginDetails.usmSection, usmSectionTemp);
  if(usmDataExists(conn, usmLoginDetails.usmUserName, "user_name", "user")){
   printf("\nUsername already exists. Choose a new one...");
   getch();
   uiDisplayMain();
  }else{
   usmInsertUser(conn,usmLoginDetails);
   printf("Sign-up success!");
   getch();
   pmViewProjects(conn, usmLoginDetails.usmUserName);
  }
}

void usmInputPassword(string20 usmPassword){
  strcpy(usmPassword," ");
  char usmInput = ' ';
  int usmLength = 0;
  do
  {     usmInput = getch();  
        if(usmInput != 13 && usmInput != 8 && usmLength < 20 && usmLength >= 0)
        {  usmPassword[usmLength] = usmInput;
           printf("*");
           usmLength++;
        }
        else if(usmInput == 8 && usmLength > 0)
        {  printf("\b"); 
           printf(" "); 
           printf("\b");
           usmPassword[usmLength] = '\0';
           usmLength--;
        }
  }while(usmInput != 13);
  for(usmLength = usmLength; usmLength < 20; usmLength++){
    usmPassword[usmLength] = '\0';
  }
}

void usmInsertUser(MYSQL* conn, usmUserAccount usmAccount){
  string500 usmQuery;
  sprintf(usmQuery, "insert into user(user_name, password, section_id) values ('%s','%s',(select section_id from section where section_name = '%s'));", 
  usmAccount.usmUserName,usmAccount.usmPassword,usmAccount.usmSection);
  mysql_query(conn,usmQuery);
  pmGenerateProjects(conn, usmAccount.usmUserName);
}

int usmDataExists(MYSQL* conn, char * basis, char * basis_column, char * basis_table){
  int usmExisted = 0;
  MYSQL_RES *res;
  MYSQL_ROW row;
  string500 usmQuery;
  sprintf(usmQuery, "select %s from %s;", basis_column, basis_table);
  mysql_query(conn,usmQuery); 
  res = mysql_use_result(conn);
  
  while((row = mysql_fetch_row(res)) != NULL){
    if(!strcmp(row[0],basis)){
      usmExisted = 1;
    }
  }
  return usmExisted;
} 
void usmChooseSections(MYSQL* conn, string3 usmSectionTemp){
  int usmLogInValid = 0;
  MYSQL_RES *res;
  MYSQL_ROW row;
  string3 usmSections[100];
  string500 usmQuery;
  sprintf(usmQuery, "select section_name from section;");
  mysql_query(conn,usmQuery); 
  res = mysql_use_result(conn);
  int i = 0;
  int usmInput;
  while((row = mysql_fetch_row(res)) != NULL){
    i++;
    printf("\n[%d] %s", i, row[0]);
    strcpy(usmSections[i-1],row[0]);
  }
  do{ 
	  printf("\n\nEnter section: ");                                  
	  scanf("%d",&usmInput);
	  if(usmInput<1 || usmInput>i){
	  printf("Sorry, invalid index...");
	  getch();
	  }
  }while(usmInput<1 || usmInput>i);
  strcpy(usmSectionTemp,usmSections[usmInput-1]);  
}

int usmVerifyAccount(MYSQL* conn, usmUserAccount usmAccount){
  int usmLogInValid = 0;
  MYSQL_RES *res;
  MYSQL_ROW row;
  
  string500 usmQuery;
  sprintf(usmQuery, "select * from user where user_name = '%s' and password = '%s';", 
  usmAccount.usmUserName,usmAccount.usmPassword);
  mysql_query(conn,usmQuery); 
  res = mysql_use_result(conn);
  
  while((row = mysql_fetch_row(res)) != NULL){
      usmLogInValid = 1;
  }
  return usmLogInValid;  
}

struct usmSectionStruct
{
  string3 usmSectionName;
  string40 usmProfessor;
};

typedef struct usmSectionStruct usmSection;

void usmInsertSection(MYSQL* conn, usmSection usmSec);

void usmAddSection(MYSQL *conn){
  usmSection usmSectionDetails;
  system("cls");
  printf("Add Section\n");
  printf("Section Name (3 chars. max):");
  scanf("%3s", usmSectionDetails.usmSectionName);
  fflush(stdin) ;
  printf("Professor Name (40 chars. max):");
  scanf("%[^\n]", usmSectionDetails.usmProfessor);
  if(usmDataExists(conn,usmSectionDetails.usmSectionName, "section_name", "section")){
    printf("\nSorry, %s already exists...\n", usmSectionDetails.usmSectionName);
    getch();
    uiSectionManagement(conn);
  }else{
    usmInsertSection(conn, usmSectionDetails);
    printf("Section Added! Succesful! ");
    getch();
    uiSectionManagement(conn);
  } 
}

void usmInsertSection(MYSQL* conn, usmSection usmSec){
  string500 usmQuery;
  sprintf(usmQuery, "insert into section(section_name, professor_name) values ('%s','%s');", 
  usmSec.usmSectionName,usmSec.usmProfessor);
  mysql_query(conn,usmQuery);
}

void usmUpdateSection(MYSQL* conn, usmSection usmSec, string30 usmSectionOrig){
  string500 usmQuery;
  sprintf(usmQuery, "update section set section_name = '%s', professor_name = '%s' where section_name = '%s';", 
  usmSec.usmSectionName,usmSec.usmProfessor, usmSectionOrig);
  mysql_query(conn,usmQuery);
}

void usmEditSection(MYSQL* conn){
  usmSection usmSectionDetails;
  system("cls");
  printf("Edit Section");
  printf("\nChoose Section:");
  string3 usmSectionName;
  usmChooseSections(conn, usmSectionName);
  system("cls");
  printf("Editing: %s", usmSectionName);
  printf("\nSection Name (3 chars. max):");
  scanf("%s", usmSectionDetails.usmSectionName);
  fflush(stdin) ;
  printf("Professor Name (40 chars. max):");
  scanf("%[^\n]", usmSectionDetails.usmProfessor);
  if(strcmp(usmSectionName,usmSectionDetails.usmSectionName) && usmDataExists(conn,usmSectionDetails.usmSectionName, "section_name", "section")){
    printf("\nSorry, %s already exists", usmSectionDetails.usmSectionName);
    getch();
    uiSectionManagement(conn);
  }else{
    usmUpdateSection(conn, usmSectionDetails, usmSectionName);
    printf("\nSection Updated! Succesful! ");
    getch();
    uiSectionManagement(conn);
  } 
}

void usmViewSection(MYSQL* conn){
  MYSQL_RES *res;
  MYSQL_ROW row;
  string500 usmQuery;
  sprintf(usmQuery, "select * from section;");
  mysql_query(conn,usmQuery); 
  res = mysql_use_result(conn);
  int usmInput;
  system("cls");
  printf("\nSection Name\tProfessor");
  while((row = mysql_fetch_row(res)) != NULL){
    printf("\n%s\t\t%s", row[1], row[2]);
  }
  
  getch();
  uiSectionManagement(conn);
}

/**PROJECT MANAGEMENT MODULE**/

void pmGenerateProjects(MYSQL *conn, string40 userName){
  string500 usmQuery;
  int i;
  for(i = 1; i <= 5; i++){
    sprintf(usmQuery, "insert into project(user_id, psp_id,current_phase,done) values ((select user_id from user where user_name = '%s'),%d, (select phase_id from phase where phase_name = 'None'),0);", 
    userName,i);
    mysql_query(conn,usmQuery);
  }  
}

void pmViewProjectDetails(MYSQL_RES *res){
  int projectStatus[5]; 
  MYSQL_ROW row;
  system("cls");
  int done = 1;
  int current = 0;
  int idle = 0;
  int projNum = 1;
  int curr = 0;
  int pmInput;
  fflush(stdin);
  int x = 0;
  while((row = mysql_fetch_row(res)) != NULL){
    projectStatus[x] = atoi(row[4]);
    x++;
  }
  
  do{
	  	done=1;
	  	curr = 0;
	  	CLEARSCR;
	  	printf("Personal Software Process\n\n");
	  	for(projNum = 1; projNum < 6; projNum++){
	  		
	    	if(projectStatus[projNum - 1] == 0 && done){
	      		done = 0;
	      		current = 1;
	    	}else if(projectStatus[projNum - 1] == 0 && current){
	      		current = 0;
	      		idle = 1;
	    	}
	    	
		    if(done){
		      printf("Project %d - done\n", projNum);
		    }else if(current){
		      curr = projNum;
		      printf("Project %d - current\n", projNum);
		    }else if(idle){
		      printf("Project %d - not yet started\n", projNum);
		    }
	  	}
	
	  
		printf("\n[1] View Done Projects\n");
		  
		if(!curr){
			printf("\nYou are currently done with everything!");
		}
		else{
		  	printf("[2] Continue project %d", curr);
		}
		printf("\n\n[0] Go Back");
		printf("\n\nchoice: ");
		scanf("%d", &pmInput);
		fflush(stdin);
	
		if(pmInput == 2){
			uiDisplayMain();//GO to current project KEVIN
		}
		else if(pmInput == 1){
		    uiDisplayMain();//CALL uiViewForms(); BERNA
		}
		else if(pmInput == 0){
			
		}  
		else{
			printf("Invalid Input, choices are 0, 1, or 2 only...");
		  	getch();
		}
	}while(pmInput != 0);
	  
	if(pmInput == 0){
	  	uiDisplayMain();
	}
}

void pmViewProjects(MYSQL *conn, string40 usmUserName){
  MYSQL_RES *res;
  string500 usmQuery;
  sprintf(usmQuery, "select * from project where user_id = (select user_id from user where user_name = '%s');", usmUserName);
  mysql_query(conn,usmQuery); 
  res = mysql_use_result(conn);
  pmViewProjectDetails(res);
}


void pmSetProjectDone(MYSQL* conn, string40 userName, int pspNum){
  string500 usmQuery;
  sprintf(usmQuery, "update project set done = 1 where user_id = (select user_id from user where user_name = '%s') and psp_id = %d;", 
  userName, pspNum);
  mysql_query(conn,usmQuery);
}

/**UI MODULE**/
void uiViewForms(){
	int choice;
	do{
		//LIST OF PROJECTS WHICH IS DONE ALREADY...
		//CHOOSE FROM LIST before the following:
		CLEARSCR();
		printf("PSP Forms:");
		printf("[1] Project Plan Summary\n");
		printf("[2] Size Estimating Template\n");
		printf("[3] Task Planning Template\n");
		printf("[4] Schedule Planning Template\n");
		printf("[5] Test Report Template\n");
		printf("[6] Process Improvement Plan\n\n");
		printf("[0] Go Back\n\n");
		printf("Which form do you want to view? ");
		scanf("&d", &choice);
		
		if(choice == 1){
			//VIEW PROJECT PLAN SUMMARY ALLEN
		}
		else if(choice == 2){
			//VIEW SIZE ESTIMATING TEMPLATE ALLEN
		}
		else if(choice == 3){
			//VIEW TASK PLANNING TEMPLATE KEVIN
		}
		else if(choice == 4){
			//VIEW SCHEDULE PLANNING TEMPLATE ALLEN
		}
		else if(choice == 5){
			//VIEW TESTREPORT TEMPLATE BERNA - trtDisplayAllReports(conn, userProjectId);
		}
		else if(choice == 6){
			//VIEW PROCESS IMPROVEMENT PLAN BERNA - pipDisplayPlan(conn, procImpPlan.piplanId);
		}
		else if(choice == 0){
			
		}
		else{
			printf("\nInvalid input, choose from the choices above only...");
			getch();
		}
	}while(choice < 0 || choice > 6); //CHECK max choice if 6
}

void uiSectionManagement(MYSQL* conn){
	int choice;
	
	do{	
		CLEARSCR;
		printf("SECTION MANAGEMENT\n\n");
		printf("[1] Add Section\n");
		printf("[2] Edit Section\n");
		printf("[3] View Section\n\n");
		
		printf("[0] Go Back\n\n");
		
		printf("choice: ");
		scanf("%d", &choice);
		
		if(choice == 1){
			usmAddSection(conn);
		}
		else if(choice == 2){
			usmEditSection(conn);
		}
		else if(choice == 3){
			usmViewSection(conn);
		}
		else if(choice == 0){
			uiDisplayMain();
		}
		else{
			printf("\n\nInvalid input...");
			getch();
		}
	}while(choice != 0 && choice != 1 && choice != 2 && choice != 3);
}

int uiDisplayMain(){
	int choice;
	
	MYSQL* conn = dbConnectDefaultDatabase("root", "p@ssword");
	
	do{
		CLEARSCR;
		printf("|-:-:-:-:-:-:-:-:-:-:-:-:-:-:-:-:-:-:-|\n");
		printf("- AUTOMATED PERSONAL SOFTWARE PROCESS -\n");
		printf("|-:-:-:-:-:-:-:-:-:-:-:-:-:-:-:-:-:-:-|\n");
		printf("	[1] Login\n");
		printf("	[2] Signup\n");
		printf("	[3] Section Management\n\n");
		
		printf("	[0] Exit\n\n");
		
		printf("	choice: ");
		scanf("%d", &choice);
		
		if(choice == 1){
			usmLogin(conn);
		}
		else if(choice == 2){
			usmSignup(conn);
		}
		else if(choice == 3){
			uiSectionManagement(conn);
		}
		else if(choice == 0){
		}
		else{
			printf("\n\nInvalid input...");
			getch();
		}
	}while(choice != 0 && choice != 1 && choice != 2 && choice != 3);
	
	dbDisconnectDatabase(conn);
	fflush(stdin);
	return 0;
}

int main() {
	//return testMain();
	//TRT: trtMenu(conn, userProjectId);
	//PIP: pipMenu(conn, userProjectId);
	
	return uiDisplayMain(); 
}
