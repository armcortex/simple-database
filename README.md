# Simple Database
A self-taught project exploring database design using C 
with Catch2 testing framework.

## Support Command
```
All Support commands: 
    help: 
    exit: 
    create: 
        Create sub-commands: 
            - database <database name> 
            - table <table name> (<column name> <column type> ...) 
    use: 
        <database name> 
    delete: 
        database <database name> 
    select: 
        select <column_names> from <table_name> (where <condition> ...)
    insert:
        insert <table_name> values <value1,value2,value3,...>
```

## Usage

### 0. Create environment
1. Build Docker Image
```
./scripts/build_docker_image.sh
```

2. Run Docker Container
```
./scripts/run_docker_container.sh
```

3. Build Project (compile with 8 cores)
```
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release -- -j 8
```

### 1. Create a database
`create database test_db`

Result:
```
create database test_db
Create database at: ../DB_DATA/test_db/test_db.json 
```

### 2. Use database
`use test_db` 

Results:
```
use test_db
Using database: test_db 
```

### 3. Create table
`create table test_table name STRING age INT height FLOAT`
- with column name `name`, `age`, `height`

Results:
```
create table test_table name STRING age INT height FLOAT
Create table at: ../DB_DATA/test_db/test_table.csv 
```

### 4. Insert data
- `insert test_table values John 30 170`
- `insert test_table values Jane 25 165` 
- `insert test_table values Alice 28 180` 
- `insert test_table values Bob 31 173` 
- `insert test_table values Charlie 29 160` 

### 5. Query data
`select name,age from test_table where age < 29 and ( name = Jane or name = Alice )`

Results:
```
name,age
Jane,25
Alice,28
```

### 6. Delete database
`delete database test_db`

Results:
```
Delete database: ../DB_DATA/test_db/test_db.json 
Delete table: ../DB_DATA/test_db/test_table.csv 
```
