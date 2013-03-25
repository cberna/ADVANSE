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

int main() {
	return testMain();
}
