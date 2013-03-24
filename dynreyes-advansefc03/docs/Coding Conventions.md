## Coding Standards
- we're using one file for everything, so while it will look unwieldy, the time we're afforded will not be enough for dealing with the fancy stuff (modular code)

- with that, we have to set a naming convention for our functions
* we're using the Hungarian notation: we use acronyms to denote the module a function belongs in before writing the actual function name

1. User & Section Management (core) (1 - usm)
2. Project Management (core) (2 - pm)
3. Phase Retrieval Interface (core) (3 - pri)
4. Time Recording Log (core) (4 - trl)
5. Defect Recording Log (5 - drl)
6. Project Plan Summary  (6 - pps)
7. Size Estimating Template (7 - st)
8. Task Planning Template (8 - tpt)
9. Schedule Planning Template (9 - spt)
10. Test Report Template (10 - trt)
11. Personal Improvement Plan (11 - pip)
12. Integration Module (12 - im)

For example, our function to convert an SQL result set to a C object for the time recording log module will be named *trlConvertToObject()*.

- We'll also use structures to define types for our tables.
* remember that time_t will be the C type used for handling time and date metadata

- We should also store the table's name and its respective column names in a global struct.

- Nothing else should be global except for the table information structures. Pass information through parameters.

- When creating a function that requires interaction with the MySQL database, always place a parameter of type MYSQL* as the first parameter.
* e.g., trlInsertLog(MYSQL* connection, TimeLog timeLog)