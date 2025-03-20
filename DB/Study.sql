--	데이터베이스 리스트 확인
--	MSSQL은 사용자가 생성하지 않아도 자동으로 생성되어 있는 시스템 데이터베이스가 있는데 master, mode, msdb, tempdb가 시스템 데이터베이스
--	사용자가 생성한 데이터베이스만 확인하고 싶은 경우 위 4가지 테이블을 제외
SELECT name FROM sys.databases WHERE name NOT IN ('master', 'mode', 'msdb', 'tempdb')

--	테이블 리스트 확인
--	현재 접속된 데이터베이스에 생성한 테이블만 확인 가능
--	각 데이터베이스마다 sys.tables라는 뷰가 생성되어 데이터베이스가 소유하고 있는 테이블 목록을 확인
--	다른 데이터베이스의 테이블을 확인하고 싶다면 [DB명].sys.tables로 조회
SELECT * FROM sys.tables;
SELECT name, OBJECT_ID FROM [MasterDB].sys.tables;

--	컬럼 리스트 확인 
SELECT * FROM sys.columns WHERE object_id = '885578193';

--	테이블 제약조건 확인
EXEC sp_help Department;

SP_HELPCONSTRAINT Department;
select * from INFORMATION_SCHEMA.CONSTRAINT_COLUMN_USAGE where table_name = 'Department'

--	현재 세션과 연결되어 있는 데이터베이스 이름 반환
SELECT DB_NAME() AS 'SelectedDatabase';

--	해당하는 데이터베이스로 컨텍스트 전환
USE Company;

--	데이터베이스 생성
CREATE DATABASE Company;

--	데이터베이스 삭제
DROP DATABASE Company;


--	constraint 이름 명시
--	이름을 붙이면 어떤 constraint을 위반했는지 쉽게 파악
--	constraint를 삭제하고 싶을 때 해당 이름으로 삭제 가능
--	오류 메세지에 제약조건의 이름이 명시되어서 무슨 이유떄문인지 파악하기 쉬워짐
CREATE TABLE TEST (
	age INT CONSTRAINT age_over_20 CHECK ( age > 20 )
);

insert into TEST (age) values (15);

--	MSSQL의 스키마는 네임 스페이스

/*

DEPARTMENT
id	name	leader_id


EMPLOYEE
id	name	birth_date	sex		position	salary	dept_id


PROJECT
id	name	leader_id	start_date	end_date


WORKS_ON
empl_id	proj_id

*/

--	primary key : table의 tuple을 식별하기 위해 사용, 하나 이상의 attribute(s)로 구성
--	primary key는 중복된 값을 가질 수 없으며, NULL도 값으로 가질 수 없다

--	unique로 지정된 attribute(s)는 중복된 값을 가질 수 없다
--	단, NULL은 중복을 허용할 수 도 있다.

--	foreign key : attribute(s)가 다른 table의 primary key나 unique key를 참조할 떄 사용

CREATE TABLE Department (
	id			INT PRIMARY KEY,
	name		NVARCHAR(20)	NOT NULL UNIQUE,
	leader_id	INT
	--	PRIMARY KEY (id, name) -> primary key가 attribute 하나 이상으로 구성될 때
	--	UNIQUE (id, name) -> unique key가 attribute 하나 이상으로 구성될 때
);

--	ALTER TABLE : TABLE의 schema를 변경하고 싶을 때 사용
ALTER TABLE Department ADD FOREIGN KEY (leader_id) REFERENCES Employee(id);

--	제약조건 삭제
--	ALTER TABLE dbo.성적표 DROP CONSTRAINT FK__성적표__출석번호__32E0915F

--	TABLE 삭제
DROP TABLE Employee;
DROP TABLE Project;

--	테이블 제약조건 확인
SP_HELPCONSTRAINT Department;

CREATE TABLE Employee (
	id			INT PRIMARY KEY,
	name		NVARCHAR(30)	NOT NULL,
	birth_date	DATE,
	--	CHECK : attribute의 값을 제한하고 싶을 떄
	sex			NVARCHAR(1)		CHECK ( sex IN ('M', 'F')),
	position	NVARCHAR(10),
	salary		INT				DEFAULT 50000000,
	dept_id		INT,

	--	FOREIGN KEY 
	--	on DELETE reference_option
	--	on UPDATE reference_option

	--	reference_option
	--	CASCADE		: 참조값의 삭제/변경을 그대로 반영
	--	SET NULL	: 참조값이 삭제/변경 시 NULL로 변경
	--	RESTRICT	: 참조값이 삭제/변경되는 것을 금지
	--	NO ACTION	: RESTRICT와 유사
	--	SET DEFAULT	: 참조값이 삭제/변경 시 DEFAULT 값으로 변경

	FOREIGN KEY (dept_id) references Department(id),
	CHECK (salary >= 50000000)
);

CREATE TABLE Project (
	id			INT PRIMARY KEY,
	name		NVARCHAR(20) NOT NULL UNIQUE,
	leader_id	INT,
	start_date	DATE,
	end_date	DATE,
	FOREIGN KEY (leader_id) references Employee(id),
	CHECK ( start_date < end_date )
);

CREATE TABLE WorksOn (
	empl_id		INT,
	proj_id		INT,
	PRIMARY KEY (empl_id, proj_id),
	FOREIGN KEY (empl_id) references Employee(id),
	FOREIGN KEY (proj_id) references Project(id)
);

SELECT * FROM Department;
SELECT * FROM Employee;

INSERT INTO Department (id, name) VALUES(1,'test');
UPDATE Department SET leader_id = 1 Where id = 1;
DELETE FROM Department WHERE id = 1;
INSERT INTO Employee (id,name,birth_date, sex, position, salary, dept_id) VALUES(1,'imsu',getdate(),'M','program',55555555,1); 

-- id가 14인 임직원보다 생일이 빠른 임직원의 id, 이름, 생일
SELECT id,name,birth_date FROM Employee
WHERE birth_date < ( SELECT birth_date FROM Employee WHERE id = 14 );

select * from Employee;
SELECT * FROM Project;
SELECT * FROM WorksOn;

-- id가 1인 임직원과 같은 부서 같은 성별인 임직원들의 id, 이름, 직군
SELECT id,name,position FROM Employee
WHERE EXISTS ( SELECT dept_id,sex FROM Employee WHERE id = 1 );

-- id가 5인 임직원과 같은 프로젝트에 참여한 임직원들의 id
SELECT DISTINCT empl_id FROM WorksOn
WHERE empl_id != 5 AND proj_id IN ( SELECT proj_id FROM WorksOn WHERE empl_id = 5);

-- id가 5인 임직원과 같은 프로젝트에 참여한 임직원들의 id와 이름
SELECT id, name FROM Employee
WHERE id IN (
SELECT DISTINCT empl_id FROM WorksOn
WHERE empl_id != 5 AND proj_id IN ( SELECT proj_id FROM WorksOn WHERE empl_id = 5) );

--	위에 다른 형태
SELECT id, name FROM Employee,
(
	SELECT DISTINCT empl_id FROM WorksOn
	WHERE empl_id != 5 AND proj_id IN 
	( 
		SELECT proj_id FROM WorksOn WHERE empl_id = 5
	)
) AS DSTNCT_E
WHERE id = DSTNCT_E.empl_id;


-- id가 7혹은 12인 임직원이 참가한 프로젝트의 id와 이름
SELECT p.id, p.name FROM Project AS p
WHERE EXISTS (
SELECT * FROM WorksOn AS w WHERE w.proj_id = p.id AND w.empl_id IN (7,12) );

-- 위에 다른 형태
SELECT p.id, p.name FROM Project AS p
WHERE id in (
SELECT proj_id FROM WorksOn AS w WHERE w.empl_id IN (7,12) );


-- 2000년대생이 없는 부서의 id와 이름
SELECT * FROM Department;
SELECT * FROM Employee;
SELECT d.id, d.name FROM Department AS d
WHERE NOT EXISTS (
	SELECT * FROM Employee AS e
	WHERE e.dept_id = d.id AND e.birth_date >= '2000-01-01'
);

-- 위에 다른 형태
SELECT d.id, d.name FROM Department AS d
WHERE d.id NOT IN (
	SELECT dept_id FROM Employee AS e
	WHERE e.birth_date >= '2000-01-01'
);

-- 리더보다 높은 연봉을 받는 부서원을 가진 리더의 ID와 이름,연봉
SELECT * FROM Department;
SELECT * FROM Employee;
SELECT * FROM Project;
SELECT * FROM WorksOn;

insert into Employee (id, name, birth_date, sex, position, salary, dept_id) VALUES(2,'sopi','2000-01-02','M','program',66666666,1);

-- 리더보다 높은 연봉을 받는 부서원을 가진 리더의 ID와 이름,연봉과 해당 부서의 최고 연봉
SELECT e.id, e.name, e.salary,
(
	SELECT MAX(salary)
	FROM Employee
	WHERE dept_id = e.dept_id
) AS dept_max_salary
FROM Department AS d,Employee AS e
WHERE d.leader_id = e.id AND e.salary < ANY (
		SELECT salary
		FROM Employee
		WHERE id<> d.leader_id AND dept_id = e.dept_id
	);

-- id가 13인 임직원과 한번도 같은 프로젝트에 참여하지 못한 임직원들의 id,이름,직군
SELECT DISTINCT e.id, e.name, e.position
FROM Employee AS e, WorksOn AS w
WHERE e.id = w.empl_id AND w.proj_id <> ALL 
(
	SELECT proj_id
	FROM WorksOn
	WHERE empl_id = 13
);

--	join : 두 개이상의 table들에 있는 데이터들을 한 번에 조회

--	id가 1인 임직원이 속한 부서 이름은?
--	implicit join : from절에는 table들만 나열하고 where절에 join condition을 명시하는 방식
SELECT d.name
FROM Department as d, Employee as e
WHERE e.id = 1 AND e.dept_id = d.id;

--	explicit join : from절에 join 키워드와 함꼐 joined table들을 명시하는 방식
SELECT d.name
FROM Department as d
INNER JOIN Employee AS e ON d.id = e.dept_id
WHERE e.id = 1;

--	INNER JOIN : 두 table에서 join condition을 만족하는 tuple들로 result table을 만드는 join
--	join condition에서 null 값을 가지는 tuple을 result table에 포함되지 못한다.

--	OUTER JOIN : 두 table에서 join condition을 만족하지 않는 tuple들도 result table에 포함하는 join
--	LEFT JOIN : RIGHT TABLE의 join condition이 만족하지 않으면 null로 표기
--	RIGHT JOIN : 반대
--	FULL OUTER : 두 TABLE 전부다

--	equi join
--	join condition = 연산자를 사용하는 join

--	natural join
--	두 table에서 같은 이름을 가지는 모든 attribute pair에 대해서 equi join을 수행

--	cross join
--	두 table에서 tuple pair로 만들 수 있는 모든 조합을 result table로 반환
--	약간 브로드캐스트 느낌?

--	self join
--	table이 자기 자신에게 join

--	id가 1003인 부서에 속하는 임직원 중 리더를 제외한 부서원의 id,이름,연봉
SELECT e.id, e.name, e.salary
FROM Employee AS e
INNER JOIN Department AS d ON d.id = e.dept_id
WHERE e.id = 1003 AND e.id != d.leader_id;

--	id가 2001인 프로젝트에 참여한 임직원들의 이름과 직군과 소속 부서
SELECT e.name, e.position, d.name
FROM WorksOn AS w	INNER JOIN Employee AS e on w.empl_id = e.id
					LEFT JOIN Department AS d ON e.dept_id = d.id
					WHERE w.proj_id = 2001;



--	Order by
--	조회 결과를 attribute 기준으로 정렬하여 가져오고 싶을 떄 사용

--	임직원들의 dept_id로 asc 기준하고 , dept_id가 같으면 salary로 desc 기준 정렬
SELECT * FROM Employee ORDER BY dept_id ASC, salary DESC;

--	aggregate function
--	여러 tuple들의 정보를 요약해서 하나의 값으로 추출
--	COUNT,SUM,MAX,MIN,AVG
--	NULL 값들은 제외하고 요약 값을 추출

--	임직원들의 수
--	NULL값을 포함해서 tuple 개수를 구하고 싶으면 *
select COUNT(dept_id) AS Count from Employee;

--	GROUP BY
--	관심있는 attribute 기준으로 그룹을 나눠서 그룹별호 aggregate function을 적용하고 싶을 때
--	grouping attribute : 그룹을나누는 기준이 되는 attribute
--	grouping attriubte에 NULL 값이 있으면 NULL 값을 가지는 tuple끼리 묶인다.

--	각 프로젝트에 참여한 임직원 수와 최대 연봉과 최소 연봉과 평균 연봉
SELECT w.proj_id, COUNT(*), MAX(salary), MIN(salary), AVG(salary)
FROM WorksOn AS w INNER JOIN Employee AS e ON w.empl_id = e.id
GROUP BY w.proj_id;

--	프로젝트에 참여 인원이 7명 이상인 프로젝트들에 대해서 각 프로젝트에 참여한 임직원 수와 최대 연봉과 최소 연봉과 평균 연봉
--	HAVING
--	aggregate function의 결과값을 바탕으로 그룹을 필터링하고 싶을 때 사용
--	HAVING절에 만족한 그룹만 결과애 나온다.
SELECT w.proj_id, COUNT(*), MAX(salary), MIN(salary), AVG(salary)
FROM WorksOn AS w INNER JOIN Employee AS e ON w.empl_id = e.id
GROUP BY w.proj_id
HAVING COUNT(*) >= 7;

--	각 부서별 인원수를 인원 수가 많은 순서대로 정렬
SELECT dept_id, COUNT(*) AS Count
FROM Employee
GROUP BY dept_id
ORDER BY Count desc; 


--	각 부서별 - 성별 인원수를 인원 수가 많은 순서대로 정렬
SELECT dept_id, sex, COUNT(*) AS Count
FROM Employee
GROUP BY dept_id, sex
ORDER BY Count desc; 

--	회사 전체 평균 연봉보다 평균 연봉이 적은 부서들의 평균 연봉
SELECT dept_id, AVG(salary)
FROM Employee
GROUP BY dept_id
HAVING AVG(salary) < ( SELECT AVG(salary) FROM Employee );

--	각 프로젝트별로 프로젝트에 참여한 90년대생들의 수와 이들의 평균 연봉
SELECT w.proj_id, COUNT(*) AS Count, ROUND(AVG(salary), 0) AS Avg
FROM WorksOn AS w
INNER JOIN Employee AS e ON w.empl_id = e.id
WHERE e.birth_date BETWEEN '1990-01-01' AND '1999-12-31'
GROUP BY w.proj_id
ORDER BY w.proj_id;

--	프로젝트 참여 인원이 7명 이상인 프로젝트에 한정해서 각 프로젝트별로 프로젝트에 참여한 90년대생들의 수와 이들의 평균 연봉
SELECT w.proj_id, COUNT(*) AS Countt, ROUND(AVG(salary), 0) AS Avg
FROM WorksOn AS w
INNER JOIN Employee AS e ON w.empl_id = e.id
WHERE e.birth_date BETWEEN '1990-01-01' AND '1999-12-31' AND 
	w.proj_id IN ( SELECT proj_id FROM WorksOn GROUP BY proj_id HAVING COUNT(*) >= 7 )
GROUP BY w.proj_id
ORDER BY w.proj_id;

--	SELECT 실행 순서
--	6. SELECT attribute or aggregate function
--	1. FROM table
--	2. WHERE condition
--	3. GROUP BY group attribute
--	4. HAVING group condition
--	5. ORDER BY attribute

--	stored function
--	사용자가 정의한 함수
--	DBMS에 저장되고 사용되는 함수

--	임직원의 ID를 열자리 정수로 랜덤하게 발급
--	ID의 맨 앞자리는 1로 고정

CREATE FUNCTION dbo.id_generator(@rand FLOAT)
RETURNS INT
AS
BEGIN
	DECLARE @retValue INT;
	SET @retValue = 1000000000 + FLOOR(@rand * 1000000000);
	RETURN @retValue;
END
GO

DROP FUNCTION dbo.id_generator;

--	함수 조회
SELECT * FROM INFORMATION_SCHEMA.ROUTINES
WHERE ROUTINE_TYPE = 'FUNCTION';

--	프로시저 조회
SELECT * FROM INFORMATION_SCHEMA.ROUTINES
WHERE ROUTINE_TYPE = 'PROCEDURE';

INSERT INTO Employee
VALUES (dbo.id_generator(RAND()), 'JEHN', '1991-08-04', 'F', 'PO', 100000000, 1); 

SELECT 1000000000 + FLOOR(RAND()* 1000000000);

SELECT * FROM Employee;
DELETE FROM Employee WHERE name = 'JEHN';


--	부서의 id를 파라미터로 받으면 해당 부서의 평균 연봉을 알려주는 함수
CREATE FUNCTION dept_avg_salary(@dept_id INT)
RETURNS INT
AS
BEGIN
	DECLARE @avg_sal INT;
	SET @avg_sal = (SELECT AVG(salary) 
			FROM Employee
			WHERE dept_id = @dept_id);
	RETURN @avg_sal;
END

select *, dbo.dept_avg_salary(id) AS AVGSALARY FROM Department;

--	loop 반복 작업
--	case 값에 따라 분기 처리
--	예외 처리

--	stored procedure
--	사용자가 정의한 프로시저
--	구체적인 하나의 task를 수행

--	두 정수의 곱셈 결과를 가져오는 

CREATE PROCEDURE dbo.product
	@a INT,
	@b INT,
	@output INT OUTPUT
AS
	BEGIN
		SELECT @output = @a * @b;
	END
GO

DECLARE @value INT;
EXEC dbo.product 1,3, @value OUTPUT;
PRINT @value;
GO

--	두 정수를 맞바꾸는 프로시저
CREATE PROCEDURE dbo.swapValue
	@a INT OUTPUT,
	@b INT OUTPUT
AS
	BEGIN
		DECLARE @temp INT;
		SET @temp = @a;
		SELECT @a = @b;
		SELECT @b = @temp;
	END
GO

DROP PROCEDURE dbo.swapValue;

DECLARE @a INT;
DECLARE @b INT;
SET @a = 1;
SET @b = 3;

EXEC dbo.swapValue @a OUTPUT, @b OUTPUT;
PRINT @a;
PRINT @b;
GO

--	각 부서별 평균 연봉을 가져오는 

CREATE PROCEDURE dbo.get_dept_avg_salary
AS
	BEGIN
		SELECT dept_id, avg(salary)
		FROM Employee
		GROUP BY dept_id;
	END
GO

EXEC dbo.get_dept_avg_salary;


CREATE TABLE Users
(
	id INT NOT NULL PRIMARY KEY,
	nickname NVARCHAR(20)
);

CREATE TABLE Nickname_logs
(
	id int not null,
	prev_nickname NVARCHAR(20),
	until	Datetime default getdate(),

	FOREIGN KEY (id) references Users(id)
);

insert into users(id, nickname) values(1, 'DINGYO');
insert into Nickname_logs(id, prev_nickname) VALUES(1, 'MESSI');

SELECT * FROM Users;
SELECT * FROM Nickname_logs;

--	사용자가 프로필 닉네임을 바꾸면 이전 닉네임을 로그에 저장하고 새 닉네임으로 업데이트하는 프로시저
CREATE PROCEDURE dbo.UpdateNickName
	@id INT,
	@newnickname NVARCHAR(20)
AS
	BEGIN
		DECLARE @prevnickname NVARCHAR(20);
		SELECT @prevnickname = ( SELECT nickname FROM Users WHERE id = @id);
		INSERT INTO Nickname_logs(id, prev_nickname, until) VALUES(@id, @prevnickname, GETDATE());
		UPDATE Users SET nickname = @newnickname WHERE id = @id;
	END
GO

EXEC dbo.UpdateNickName 1, 'MINAMI'

--	비즈니스 로직 -> stored procedure
--	장점
--	network traffic을 줄여서 응답 속도를 향상시킬 수 있다.
--	여러 서비스 재사용 가능
--	민감한 정보에 대한 접근을 제한

--	단점
--	유지 관리 보수 비용이 커진다.

--	trigger
--	데이터에 변경이 생겼을 떄 
--	DB Insert, Update, Delete가 발생햇을 때
--	자동적으로 실행되는 프로시저

SELECT * FROM Users;
SELECT * FROM Nickname_logs;

CREATE TRIGGER log_user_nickname_trigger
ON Users
FOR UPDATE
AS
BEGIN
	SELECT 'Deleted => ', * FROM deleted;
	SELECT 'Inserted => ', * FROM inserted;	--	NEW DATA
	INSERT INTO Nickname_logs(id,prev_nickname) SELECT id, nickname FROM deleted;
END

DROP TRIGGER dbo.Sum_buy_prices_trigger;

UPDATE Users SET nickname = 'SENA' WHERE id = 1;
DELETE FROM Nickname_logs WHERE prev_nickname = 'SENA'

CREATE TABLE Buy(
	id INT NOT NULL PRIMARY KEY,
	userId INT NOT NULL,
	price INT
	
--	FOREIGN KEY (userid) References User_buy_stats(id)
);

CREATE TABLE User_buy_stats(
	userId INT NOT NULL PRIMARY KEY,
	price_sum INT NOT NULL
);

SELECT * FROM Buy;
SELECT * FROM User_buy_stats;

ALTER TABLE dbo.Buy ADD buy_at DateTime DEFAULT GETDATE();

CREATE TRIGGER Sum_buy_prices_trigger
ON Buy
FOR INSERT 
AS
BEGIN
	SELECT 'Deleted =>', * FROM deleted;
	SELECT 'Inserted =>', * FROM inserted;
	DECLARE @sum INT;
	DECLARE @userId INT;
	SET @userId = (SELECT userID FROM inserted);
	SET @sum = (SELECT SUM(Buy.price) FROM Buy,inserted WHERE Buy.userId = inserted.userId);

	IF EXISTS(
		SELECT * FROM User_buy_stats 
		WHERE userId = @userId
	)
		BEGIN
			UPDATE User_buy_stats SET price_sum = @sum FROM inserted WHERE User_buy_stats.userId = inserted.userId;
		END
	ELSE
		BEGIN
			INSERT INTO User_buy_stats(userId, price_sum) VALUES(@userId, @sum);
		END

END

INSERT INTO BUY(id, userId, price) VALUES(3, 10,7000);

--	단점
--	소스코드처럼 눈에 보이지 않아 가시적이지 않아서 개발도 관리도 문제파악도 힘들다
--	트리거에 대한 이벤트를 많이 호출하면 파악하기 힘들다. -> 자동적으로 이벤트를 발생하므로 파악 힘듬
--	과도한 사용은 DB에 부담을 주고 응답을 느리게 만든다.
--	디버깅 까다롭..
--	


--	트랜잭션
--	단일한 논리적입 작업 단위
--	논리적인 이유로 여러 SQL문들을 단일 작업으로 묶어서 나눠질 수 없게 만든 것
--	transaction의 SQL문들 중에 일부만 성공해서 DB에 반영되는 일은 없다.

CREATE TABLE Account(
	id INT NOT NULL PRIMARY KEY,
	balance INT
);

SELECT * FROM Account;
INSERT INTO Account(id, balance) VALUES(1, 1000000);
INSERT INTO Account(id, balance) VALUES(2, 2000000);

BEGIN TRAN
	BEGIN
		UPDATE Account SET balance = balance - 200000 WHERE id = '1';
		UPDATE Account SET balance = balance + 200000 WHERE id = '2';
	END
COMMIT TRAN
--	COMMIT : 지금까지 작업한 내용을 DB에 영구적으로 저장 , transaction을 종료

BEGIN TRAN
	BEGIN
		UPDATE Account SET balance = balance - 300000 WHERE id = '1';
	END
COMMIT TRAN

BEGIN TRAN
ROLLBACK TRAN
--	ROLLBACK : 지금까지 작업들을 모두 취소하고 TRANSACTION 이전 상태로 되돌린다 , transaction을 종료

--	@@AUTOCOMMIT
--	각각의 SQL문들을 자동으로 TRANSACTION 처리
--	SQL문이 성공적으로 실행하면 자동으로  COMMIT
--	실쟁 중에 문제가 있으면 알아서 ROLLBACK
--	BEGINT TRAN시 autocommit은 비활성화 , tran 종료시 다시 활성화

SET IMPLICIT_TRANSACTIONS ON
SET IMPLICIT_TRANSACTIONS OFF

DELETE FROM Account WHERE balance <= 1000000;

--	특징
--	1. Atomicity 원자성
--	논리적으로 쪼개질 수 없는 작업 단위이기 때문에 내부의 SQL문들이 모두 성공
--	중간에 SQL문이 실패하면 지금까지의 작업을 모두 취소하여 아무 일도 없었던 것처럼 rollback

--	commit 실행 시 db에 영구적으로 저장하는 것은 dbms가 담당
--	rollback 실행 시 이전 상태로 되돌리는 것도 dbms가 담당
--	commit과 rollback을 언제 할지는 개발자

--	2. Consistency 일관성
--	constraints, trigger 등을 통해 DB에 정의된 rules을 transaction이 위반했다면 rollback 해야 한다.
--	transaction이 DB에 정의된 rule을 위반했는지 DBMS가 commit 전에 확인하고 알려준다.

--	3. Isolation 격리 분리
--	여러 transaction들이 동시에 실행될 때도 혼자 실행되는 것처럼 동작
--	DBMS는 여러 종류의 Isolation level을 제공
--	부하가 있을 수 있다.

--	4. Durability 영존성
--	commit된 transaction은 DB에 영구적으로 저장
--	DB System에 문제가 생격도 commit된 transaction은 DB에 남는다.
--	비휘발성 메모리에 저장됨

--	Schedule
--	여러 transaction들이 동시에 실핼될 때
--	각 transaction에 속한 opertions들의 실행 순서
--	각 transaction내의 operations들의 순서는 바뀌지 않는다.
--	r1(k)-w1(k)

--	Serial schedule
--	transaction들이 겹치지 않고 한번에 하나씩 실행되는 Schedule
--	한번에 하나의 transaction만 실행되기 때문에 좋은 성능을 기대하기는 어렵다

--	Nonserial schedule
--	transaction들이 겹처서 실행되는 Schedule
--	transaction들이 겹처서 실행되기 때문에 동시성이 높아져서 같은 시간 동안 더 많은 transaction들을 처리할 수 있다.
--	transaction들이 어떤 형태로 겹쳐서 실행되는지에 따라 이상한 결과가 나 올 수 있다 -> 멀티스레드 비슷하네연

--	Nonserial schedule로 실행해도 이상한 결과가 나오지 않을 수 있는 방법이 있을까?
--	serial schedule과 동일한 nonserial schedule을 실행하면 되겠다?
--	schedule이 동일하다?

--	Confilct
--	of two operations
--	세 가지 조건을 모두 만족하면 Confilct
--	1. 서로 다른 transaction 소속
--	2. 같은 데이터에 접근
--	3. 최소 하나는 write operation
--	read-write confilct
--	write-write confilct
--	순서가 바뀌면 결과도 바뀐다.

--	Confilct equivalent
--	for two schedules
--	두 조건 모두 만족하면 confilct equivalent
--	1. 두 schedule은 같은 transaction들을 가진다.
--	2. 어떤 confilcing operations의 순서도 양쪽 schedule 모두 동일하다
--	serial schedule과 confilct equivalent일 때 -> Confict serializable -> 이상한 결과X
--	confilct serializable한 nonserial schedule을 허용
--	구현
--	여러 transaction을 동시에 실행해도 schedule이 confilct serializable하도록 보장하는 프로토콜을 적용

--	concurrency control이 어떤 schedule도 serializable하게 만든다
--	이거와 관련 있는 트랜잭션 속성이 Isolation이다
--	너무 완벽하게 할려고 하면 성능이슈가 있다.
--	그래서 isolation level 이 생겼다. ( 좀 더 기준을 내릴지 올릴지 ) 

--	unrecoverable schedule
--	schedule내에서 commit된 transaction이 rollback된 transaction의 write 했었던 데이터를 읽은 경우
--	rollback을 해도 이전 상태로 회복 불가능할 수 있기 떄문에 이런 schedule은 DBMS가 허용하면 안된다.

--	recoverable schedule
--	schedule 내에서 그 어떤 transaction도 자신이 읽은 데이터를 write한 transaction이 먼저 commit/rollback 전까지는 commit 하지 않는 경우

--	cascading rollback
--	하나의 transaction이 rollback하면 의존성이 있는 다른 transaction도 rollback 해야한다.
--	여러 transaction의 rollback이 연쇄적으로 일어나면 처리비용이 많이 든다.

--	cascadeless schedule ( avoid cascading rollback )
--	schedule내에서 어떤 transaction도 commit 되지 않는 transaction들이 write한 데이터는 읽지 않는 경우

--	strict schedule
--	schedule내에서 어떤 transaction도 commit 되지 않는 transaction들이 write한 데이터를 쓰지도 읽지도 않는 경우
--	rollback할 때 recovery가 쉽다. transaction 이전 상태로 돌려놓기만 하면 된다.


CREATE DATABASE designDB;

-- 엔티티(테이블)간의 관계를 정의한 다이어그램 (ERD) -> 데이터베이스 다이어그램

use designDB;

SELECT * FROM Company;
SELECT * FROM Department;

INSERT INTO Company VALUES(N'애플');
INSERT INTO Company VALUES(N'엔비디아');
INSERT INTO Department VALUES(1,N'개발팀',1);
INSERT INTO Department VALUES(2,N'영업팀',1);
INSERT INTO Department VALUES(1,N'디자인팀',2);
INSERT INTO Department (DepartmentId, DepartmentName) VALUES(2,N'아트팀');

UPDATE Department SET CompanyId = 2 WHERE DepartmentId = 2;

--	컬럼 이름 변경
sp_rename 'Department.[CompnayId]', N'CompanyId', 'COLUMN'
--	컬럼 타입 변경
ALTER TABLE Department ALTER COLUMN DepartmentName nvarchar(50);

SELECT * FROM Company AS C
LEFT JOIN Department AS D ON C.CompanyId = D.CompanyId
ORDER BY C.CompanyId, D.DepartmentId

DELETE FROM Department;

ALTER TABLE Department ADD CONSTRAINT Department_PK PRIMARY KEY (DepartmentId, CompanyId)

CREATE TABLE TestDepartment (
	departmentId INT PRIMARY KEY NOT NULL,
	departmentName NVARCHAR(50),
	highDepartmentId INT FOREIGN KEY REFERENCES TestDepartment(departmentId),
);

SELECT * FROM TestDepartment;

DELETE FROM TestDepartment WHERE departmentId = 3;
--	1:M 재귀 관계
--	최상위
INSERT INTO TestDepartment VALUES(1, '개발팀', NULL);
INSERT INTO TestDepartment VALUES(2, '개발1팀', 1);
INSERT INTO TestDepartment VALUES(3, '개발2팀', 1);
INSERT INTO TestDepartment VALUES(4, '개발1-1팀', 2);
INSERT INTO TestDepartment VALUES(5, '개발1-2팀', 2);

SELECT * FROM TestDepartment AS A
INNER JOIN ( SELECT * FROM TestDepartment ) AS B ON A.departmentId = B.highDepartmentId
WHERE A.departmentId = 2

--  ALTER TABLE [테이블명] DROP CONSTRAINT [FOREIGN KEY명]

ALTER TABLE TestDepartment ADD CONSTRAINT Department_Company_FK FOREIGN KEY (CompanyId) REFERENCES Company(CompanyId);

-- M:N 관계
-- 비지니스 관계?
-- 학생 <-> 과목 PK 중복 발생
-- 학생 과목은 관계가 없는 별개의 테이블이다. 마스터 테이블
-- 학생 과목 수강 
-- 하나의 테이블은 하나의 객체나 사물을 모델링!
-- 관계를 유심히


CREATE TABLE 서고 (
	서고id int PRIMARY KEY NOT NULL,
)

CREATE TABLE 서가 (
	서가id int PRIMARY KEY NOT NULL,
	서고id int 
)

ALTER TABLE 서가 ADD CONSTRAINT 서가_서고_FK FOREIGN KEY (서고id) REFERENCES 서고(서고id);

CREATE TABLE 책 (
	책id int PRIMARY KEY NOT NULL,
)

CREATE TABLE 고객 (
	고객id int PRIMARY KEY NOT NULL,
	고객명 NVARCHAR(50) NOT NULL
)

CREATE TABLE 도서목록 (
	도서목록id int PRIMARY KEY NOT NULL,
)

use 성적관리;
 
SELECT * FROM TB_SchoolGrade;
SELECT * FROM TB_SchoolClass;

INSERT INTO TB_SchoolGrade VALUES(1, '1학년'), (2, '2학년'), (3, '3학년');

CREATE PROCEDURE SP_Insert_SchoolClass
	@SchoolGradeId Tinyint,
	@SchoolClassId Tinyint,
	@SchoolClassName varchar(50)
AS
BEGIN
	SET NOCOUNT ON
	INSERT INTO TB_SchoolClass (SchoolGradeId, SchoolClassId, SchoolClassName)
	VALUES(@SchoolGradeId, @SchoolClassId, @SchoolClassName);
END
GO

EXEC SP_Insert_SchoolClass 3, 3, '3반'

USE designDB;

SELECT * FROM 신랑
SELECT * FROM 신부
SELECT * FROM 부부
INSERT INTO 신부 VALUES(1, '어우동', NULL),(2,'황진이',NULL)

UPDATE 신랑 SET 신랑명 = '박길동' WHERE 신랑id = 3;
UPDATE 신부 SET 신랑ID = 3 WHERE 신부id = 1;
UPDATE 신랑 SET 신부ID = 1 WHERE 신랑id = 3;

SELECT * FROM 신랑 AS A
FULL OUTER JOIN 신부 AS B ON A.신랑id = B.신랑id

INSERT INTO 부부 VALUES(1, '홍길동', '어우동'),(2, '김길동', '황진이')

SELECT A.신랑id 신랑신부id, a.신랑명, b.신부명
from 신랑1 a join 신부1 b on a.신랑id = b.신부id