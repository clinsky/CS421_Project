# CS421 Course Project
An implementation of a database management system.
## Compilation
Compile with make.
## Phase 1 Resubmission
We fixed the following issues with phase 1:
- "select * from foo" now shows an empty table when foo is empty.
- Select output is now formatted to see columns.
- Inserting more than one value at a time now works.
- A double can no longer be inserted for an int.
## Phase 2 Submission Notes
### create table
Please use create table in the following manner:

create table foo(\
baz integer primarykey,\
bar double notnull,\
bazzle char(10) unique notnull);

Where attributes are seperated by commas, constraints are not seperated by commas, and the closing parentheses and semi-colon appear on the same line as the last attribute. 
### insert
Please use insert in the following manner:

insert into foo values\
(20 2.2 "hi"),\
(21 12.2 "hi");

Where records are seperated by commas, attributes are not seperated by commas, and the semi-colon appears on the same line as the last record.