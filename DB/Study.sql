--	�����ͺ��̽� ����Ʈ Ȯ��
--	MSSQL�� ����ڰ� �������� �ʾƵ� �ڵ����� �����Ǿ� �ִ� �ý��� �����ͺ��̽��� �ִµ� master, mode, msdb, tempdb�� �ý��� �����ͺ��̽�
--	����ڰ� ������ �����ͺ��̽��� Ȯ���ϰ� ���� ��� �� 4���� ���̺��� ����
SELECT name FROM sys.databases WHERE name NOT IN ('master', 'mode', 'msdb', 'tempdb')

--	���̺� ����Ʈ Ȯ��
--	���� ���ӵ� �����ͺ��̽��� ������ ���̺� Ȯ�� ����
--	�� �����ͺ��̽����� sys.tables��� �䰡 �����Ǿ� �����ͺ��̽��� �����ϰ� �ִ� ���̺� ����� Ȯ��
--	�ٸ� �����ͺ��̽��� ���̺��� Ȯ���ϰ� �ʹٸ� [DB��].sys.tables�� ��ȸ
SELECT * FROM sys.tables;
SELECT name, OBJECT_ID FROM [MasterDB].sys.tables;

--	�÷� ����Ʈ Ȯ�� 
SELECT * FROM sys.columns WHERE object_id = '885578193';

--	���̺� �������� Ȯ��
EXEC sp_help Department;

SP_HELPCONSTRAINT Department;
select * from INFORMATION_SCHEMA.CONSTRAINT_COLUMN_USAGE where table_name = 'Department'

--	���� ���ǰ� ����Ǿ� �ִ� �����ͺ��̽� �̸� ��ȯ
SELECT DB_NAME() AS 'SelectedDatabase';

--	�ش��ϴ� �����ͺ��̽��� ���ؽ�Ʈ ��ȯ
USE Company;

--	�����ͺ��̽� ����
CREATE DATABASE Company;

--	�����ͺ��̽� ����
DROP DATABASE Company;


--	constraint �̸� ���
--	�̸��� ���̸� � constraint�� �����ߴ��� ���� �ľ�
--	constraint�� �����ϰ� ���� �� �ش� �̸����� ���� ����
--	���� �޼����� ���������� �̸��� ��õǾ ���� ������������ �ľ��ϱ� ������
CREATE TABLE TEST (
	age INT CONSTRAINT age_over_20 CHECK ( age > 20 )
);

insert into TEST (age) values (15);

--	MSSQL�� ��Ű���� ���� �����̽�

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

--	primary key : table�� tuple�� �ĺ��ϱ� ���� ���, �ϳ� �̻��� attribute(s)�� ����
--	primary key�� �ߺ��� ���� ���� �� ������, NULL�� ������ ���� �� ����

--	unique�� ������ attribute(s)�� �ߺ��� ���� ���� �� ����
--	��, NULL�� �ߺ��� ����� �� �� �ִ�.

--	foreign key : attribute(s)�� �ٸ� table�� primary key�� unique key�� ������ �� ���

CREATE TABLE Department (
	id			INT PRIMARY KEY,
	name		NVARCHAR(20)	NOT NULL UNIQUE,
	leader_id	INT
	--	PRIMARY KEY (id, name) -> primary key�� attribute �ϳ� �̻����� ������ ��
	--	UNIQUE (id, name) -> unique key�� attribute �ϳ� �̻����� ������ ��
);

--	ALTER TABLE : TABLE�� schema�� �����ϰ� ���� �� ���
ALTER TABLE Department ADD FOREIGN KEY (leader_id) REFERENCES Employee(id);

--	�������� ����
--	ALTER TABLE dbo.����ǥ DROP CONSTRAINT FK__����ǥ__�⼮��ȣ__32E0915F

--	TABLE ����
DROP TABLE Employee;
DROP TABLE Project;

--	���̺� �������� Ȯ��
SP_HELPCONSTRAINT Department;

CREATE TABLE Employee (
	id			INT PRIMARY KEY,
	name		NVARCHAR(30)	NOT NULL,
	birth_date	DATE,
	--	CHECK : attribute�� ���� �����ϰ� ���� ��
	sex			NVARCHAR(1)		CHECK ( sex IN ('M', 'F')),
	position	NVARCHAR(10),
	salary		INT				DEFAULT 50000000,
	dept_id		INT,

	--	FOREIGN KEY 
	--	on DELETE reference_option
	--	on UPDATE reference_option

	--	reference_option
	--	CASCADE		: �������� ����/������ �״�� �ݿ�
	--	SET NULL	: �������� ����/���� �� NULL�� ����
	--	RESTRICT	: �������� ����/����Ǵ� ���� ����
	--	NO ACTION	: RESTRICT�� ����
	--	SET DEFAULT	: �������� ����/���� �� DEFAULT ������ ����

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

-- id�� 14�� ���������� ������ ���� �������� id, �̸�, ����
SELECT id,name,birth_date FROM Employee
WHERE birth_date < ( SELECT birth_date FROM Employee WHERE id = 14 );

select * from Employee;
SELECT * FROM Project;
SELECT * FROM WorksOn;

-- id�� 1�� �������� ���� �μ� ���� ������ ���������� id, �̸�, ����
SELECT id,name,position FROM Employee
WHERE EXISTS ( SELECT dept_id,sex FROM Employee WHERE id = 1 );

-- id�� 5�� �������� ���� ������Ʈ�� ������ ���������� id
SELECT DISTINCT empl_id FROM WorksOn
WHERE empl_id != 5 AND proj_id IN ( SELECT proj_id FROM WorksOn WHERE empl_id = 5);

-- id�� 5�� �������� ���� ������Ʈ�� ������ ���������� id�� �̸�
SELECT id, name FROM Employee
WHERE id IN (
SELECT DISTINCT empl_id FROM WorksOn
WHERE empl_id != 5 AND proj_id IN ( SELECT proj_id FROM WorksOn WHERE empl_id = 5) );

--	���� �ٸ� ����
SELECT id, name FROM Employee,
(
	SELECT DISTINCT empl_id FROM WorksOn
	WHERE empl_id != 5 AND proj_id IN 
	( 
		SELECT proj_id FROM WorksOn WHERE empl_id = 5
	)
) AS DSTNCT_E
WHERE id = DSTNCT_E.empl_id;


-- id�� 7Ȥ�� 12�� �������� ������ ������Ʈ�� id�� �̸�
SELECT p.id, p.name FROM Project AS p
WHERE EXISTS (
SELECT * FROM WorksOn AS w WHERE w.proj_id = p.id AND w.empl_id IN (7,12) );

-- ���� �ٸ� ����
SELECT p.id, p.name FROM Project AS p
WHERE id in (
SELECT proj_id FROM WorksOn AS w WHERE w.empl_id IN (7,12) );


-- 2000������ ���� �μ��� id�� �̸�
SELECT * FROM Department;
SELECT * FROM Employee;
SELECT d.id, d.name FROM Department AS d
WHERE NOT EXISTS (
	SELECT * FROM Employee AS e
	WHERE e.dept_id = d.id AND e.birth_date >= '2000-01-01'
);

-- ���� �ٸ� ����
SELECT d.id, d.name FROM Department AS d
WHERE d.id NOT IN (
	SELECT dept_id FROM Employee AS e
	WHERE e.birth_date >= '2000-01-01'
);

-- �������� ���� ������ �޴� �μ����� ���� ������ ID�� �̸�,����
SELECT * FROM Department;
SELECT * FROM Employee;
SELECT * FROM Project;
SELECT * FROM WorksOn;

insert into Employee (id, name, birth_date, sex, position, salary, dept_id) VALUES(2,'sopi','2000-01-02','M','program',66666666,1);

-- �������� ���� ������ �޴� �μ����� ���� ������ ID�� �̸�,������ �ش� �μ��� �ְ� ����
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

-- id�� 13�� �������� �ѹ��� ���� ������Ʈ�� �������� ���� ���������� id,�̸�,����
SELECT DISTINCT e.id, e.name, e.position
FROM Employee AS e, WorksOn AS w
WHERE e.id = w.empl_id AND w.proj_id <> ALL 
(
	SELECT proj_id
	FROM WorksOn
	WHERE empl_id = 13
);

--	join : �� ���̻��� table�鿡 �ִ� �����͵��� �� ���� ��ȸ

--	id�� 1�� �������� ���� �μ� �̸���?
--	implicit join : from������ table�鸸 �����ϰ� where���� join condition�� ����ϴ� ���
SELECT d.name
FROM Department as d, Employee as e
WHERE e.id = 1 AND e.dept_id = d.id;

--	explicit join : from���� join Ű����� �Բ� joined table���� ����ϴ� ���
SELECT d.name
FROM Department as d
INNER JOIN Employee AS e ON d.id = e.dept_id
WHERE e.id = 1;

--	INNER JOIN : �� table���� join condition�� �����ϴ� tuple��� result table�� ����� join
--	join condition���� null ���� ������ tuple�� result table�� ���Ե��� ���Ѵ�.

--	OUTER JOIN : �� table���� join condition�� �������� �ʴ� tuple�鵵 result table�� �����ϴ� join
--	LEFT JOIN : RIGHT TABLE�� join condition�� �������� ������ null�� ǥ��
--	RIGHT JOIN : �ݴ�
--	FULL OUTER : �� TABLE ���δ�

--	equi join
--	join condition = �����ڸ� ����ϴ� join

--	natural join
--	�� table���� ���� �̸��� ������ ��� attribute pair�� ���ؼ� equi join�� ����

--	cross join
--	�� table���� tuple pair�� ���� �� �ִ� ��� ������ result table�� ��ȯ
--	�ణ ��ε�ĳ��Ʈ ����?

--	self join
--	table�� �ڱ� �ڽſ��� join

--	id�� 1003�� �μ��� ���ϴ� ������ �� ������ ������ �μ����� id,�̸�,����
SELECT e.id, e.name, e.salary
FROM Employee AS e
INNER JOIN Department AS d ON d.id = e.dept_id
WHERE e.id = 1003 AND e.id != d.leader_id;

--	id�� 2001�� ������Ʈ�� ������ ���������� �̸��� ������ �Ҽ� �μ�
SELECT e.name, e.position, d.name
FROM WorksOn AS w	INNER JOIN Employee AS e on w.empl_id = e.id
					LEFT JOIN Department AS d ON e.dept_id = d.id
					WHERE w.proj_id = 2001;



--	Order by
--	��ȸ ����� attribute �������� �����Ͽ� �������� ���� �� ���

--	���������� dept_id�� asc �����ϰ� , dept_id�� ������ salary�� desc ���� ����
SELECT * FROM Employee ORDER BY dept_id ASC, salary DESC;

--	aggregate function
--	���� tuple���� ������ ����ؼ� �ϳ��� ������ ����
--	COUNT,SUM,MAX,MIN,AVG
--	NULL ������ �����ϰ� ��� ���� ����

--	���������� ��
--	NULL���� �����ؼ� tuple ������ ���ϰ� ������ *
select COUNT(dept_id) AS Count from Employee;

--	GROUP BY
--	�����ִ� attribute �������� �׷��� ������ �׷캰ȣ aggregate function�� �����ϰ� ���� ��
--	grouping attribute : �׷��������� ������ �Ǵ� attribute
--	grouping attriubte�� NULL ���� ������ NULL ���� ������ tuple���� ���δ�.

--	�� ������Ʈ�� ������ ������ ���� �ִ� ������ �ּ� ������ ��� ����
SELECT w.proj_id, COUNT(*), MAX(salary), MIN(salary), AVG(salary)
FROM WorksOn AS w INNER JOIN Employee AS e ON w.empl_id = e.id
GROUP BY w.proj_id;

--	������Ʈ�� ���� �ο��� 7�� �̻��� ������Ʈ�鿡 ���ؼ� �� ������Ʈ�� ������ ������ ���� �ִ� ������ �ּ� ������ ��� ����
--	HAVING
--	aggregate function�� ������� �������� �׷��� ���͸��ϰ� ���� �� ���
--	HAVING���� ������ �׷츸 ����� ���´�.
SELECT w.proj_id, COUNT(*), MAX(salary), MIN(salary), AVG(salary)
FROM WorksOn AS w INNER JOIN Employee AS e ON w.empl_id = e.id
GROUP BY w.proj_id
HAVING COUNT(*) >= 7;

--	�� �μ��� �ο����� �ο� ���� ���� ������� ����
SELECT dept_id, COUNT(*) AS Count
FROM Employee
GROUP BY dept_id
ORDER BY Count desc; 


--	�� �μ��� - ���� �ο����� �ο� ���� ���� ������� ����
SELECT dept_id, sex, COUNT(*) AS Count
FROM Employee
GROUP BY dept_id, sex
ORDER BY Count desc; 

--	ȸ�� ��ü ��� �������� ��� ������ ���� �μ����� ��� ����
SELECT dept_id, AVG(salary)
FROM Employee
GROUP BY dept_id
HAVING AVG(salary) < ( SELECT AVG(salary) FROM Employee );

--	�� ������Ʈ���� ������Ʈ�� ������ 90�������� ���� �̵��� ��� ����
SELECT w.proj_id, COUNT(*) AS Count, ROUND(AVG(salary), 0) AS Avg
FROM WorksOn AS w
INNER JOIN Employee AS e ON w.empl_id = e.id
WHERE e.birth_date BETWEEN '1990-01-01' AND '1999-12-31'
GROUP BY w.proj_id
ORDER BY w.proj_id;

--	������Ʈ ���� �ο��� 7�� �̻��� ������Ʈ�� �����ؼ� �� ������Ʈ���� ������Ʈ�� ������ 90�������� ���� �̵��� ��� ����
SELECT w.proj_id, COUNT(*) AS Countt, ROUND(AVG(salary), 0) AS Avg
FROM WorksOn AS w
INNER JOIN Employee AS e ON w.empl_id = e.id
WHERE e.birth_date BETWEEN '1990-01-01' AND '1999-12-31' AND 
	w.proj_id IN ( SELECT proj_id FROM WorksOn GROUP BY proj_id HAVING COUNT(*) >= 7 )
GROUP BY w.proj_id
ORDER BY w.proj_id;

--	SELECT ���� ����
--	6. SELECT attribute or aggregate function
--	1. FROM table
--	2. WHERE condition
--	3. GROUP BY group attribute
--	4. HAVING group condition
--	5. ORDER BY attribute

--	stored function
--	����ڰ� ������ �Լ�
--	DBMS�� ����ǰ� ���Ǵ� �Լ�

--	�������� ID�� ���ڸ� ������ �����ϰ� �߱�
--	ID�� �� ���ڸ��� 1�� ����

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

--	�Լ� ��ȸ
SELECT * FROM INFORMATION_SCHEMA.ROUTINES
WHERE ROUTINE_TYPE = 'FUNCTION';

--	���ν��� ��ȸ
SELECT * FROM INFORMATION_SCHEMA.ROUTINES
WHERE ROUTINE_TYPE = 'PROCEDURE';

INSERT INTO Employee
VALUES (dbo.id_generator(RAND()), 'JEHN', '1991-08-04', 'F', 'PO', 100000000, 1); 

SELECT 1000000000 + FLOOR(RAND()* 1000000000);

SELECT * FROM Employee;
DELETE FROM Employee WHERE name = 'JEHN';


--	�μ��� id�� �Ķ���ͷ� ������ �ش� �μ��� ��� ������ �˷��ִ� �Լ�
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

--	loop �ݺ� �۾�
--	case ���� ���� �б� ó��
--	���� ó��

--	stored procedure
--	����ڰ� ������ ���ν���
--	��ü���� �ϳ��� task�� ����

--	�� ������ ���� ����� �������� 

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

--	�� ������ �¹ٲٴ� ���ν���
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

--	�� �μ��� ��� ������ �������� 

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

--	����ڰ� ������ �г����� �ٲٸ� ���� �г����� �α׿� �����ϰ� �� �г������� ������Ʈ�ϴ� ���ν���
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

--	����Ͻ� ���� -> stored procedure
--	����
--	network traffic�� �ٿ��� ���� �ӵ��� ����ų �� �ִ�.
--	���� ���� ���� ����
--	�ΰ��� ������ ���� ������ ����

--	����
--	���� ���� ���� ����� Ŀ����.

--	trigger
--	�����Ϳ� ������ ������ �� 
--	DB Insert, Update, Delete�� �߻����� ��
--	�ڵ������� ����Ǵ� ���ν���

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

--	����
--	�ҽ��ڵ�ó�� ���� ������ �ʾ� ���������� �ʾƼ� ���ߵ� ������ �����ľǵ� �����
--	Ʈ���ſ� ���� �̺�Ʈ�� ���� ȣ���ϸ� �ľ��ϱ� �����. -> �ڵ������� �̺�Ʈ�� �߻��ϹǷ� �ľ� ����
--	������ ����� DB�� �δ��� �ְ� ������ ������ �����.
--	����� ��ٷ�..
--	


--	Ʈ�����
--	������ ������ �۾� ����
--	������ ������ ���� SQL������ ���� �۾����� ��� ������ �� ���� ���� ��
--	transaction�� SQL���� �߿� �Ϻθ� �����ؼ� DB�� �ݿ��Ǵ� ���� ����.

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
--	COMMIT : ���ݱ��� �۾��� ������ DB�� ���������� ���� , transaction�� ����

BEGIN TRAN
	BEGIN
		UPDATE Account SET balance = balance - 300000 WHERE id = '1';
	END
COMMIT TRAN

BEGIN TRAN
ROLLBACK TRAN
--	ROLLBACK : ���ݱ��� �۾����� ��� ����ϰ� TRANSACTION ���� ���·� �ǵ����� , transaction�� ����

--	@@AUTOCOMMIT
--	������ SQL������ �ڵ����� TRANSACTION ó��
--	SQL���� ���������� �����ϸ� �ڵ�����  COMMIT
--	���� �߿� ������ ������ �˾Ƽ� ROLLBACK
--	BEGINT TRAN�� autocommit�� ��Ȱ��ȭ , tran ����� �ٽ� Ȱ��ȭ

SET IMPLICIT_TRANSACTIONS ON
SET IMPLICIT_TRANSACTIONS OFF

DELETE FROM Account WHERE balance <= 1000000;

--	Ư¡
--	1. Atomicity ���ڼ�
--	�������� �ɰ��� �� ���� �۾� �����̱� ������ ������ SQL������ ��� ����
--	�߰��� SQL���� �����ϸ� ���ݱ����� �۾��� ��� ����Ͽ� �ƹ� �ϵ� ������ ��ó�� rollback

--	commit ���� �� db�� ���������� �����ϴ� ���� dbms�� ���
--	rollback ���� �� ���� ���·� �ǵ����� �͵� dbms�� ���
--	commit�� rollback�� ���� ������ ������

--	2. Consistency �ϰ���
--	constraints, trigger ���� ���� DB�� ���ǵ� rules�� transaction�� �����ߴٸ� rollback �ؾ� �Ѵ�.
--	transaction�� DB�� ���ǵ� rule�� �����ߴ��� DBMS�� commit ���� Ȯ���ϰ� �˷��ش�.

--	3. Isolation �ݸ� �и�
--	���� transaction���� ���ÿ� ����� ���� ȥ�� ����Ǵ� ��ó�� ����
--	DBMS�� ���� ������ Isolation level�� ����
--	���ϰ� ���� �� �ִ�.

--	4. Durability ������
--	commit�� transaction�� DB�� ���������� ����
--	DB System�� ������ ���ݵ� commit�� transaction�� DB�� ���´�.
--	���ֹ߼� �޸𸮿� �����

--	Schedule
--	���� transaction���� ���ÿ� ���۵� ��
--	�� transaction�� ���� opertions���� ���� ����
--	�� transaction���� operations���� ������ �ٲ��� �ʴ´�.
--	r1(k)-w1(k)

--	Serial schedule
--	transaction���� ��ġ�� �ʰ� �ѹ��� �ϳ��� ����Ǵ� Schedule
--	�ѹ��� �ϳ��� transaction�� ����Ǳ� ������ ���� ������ ����ϱ�� ��ƴ�

--	Nonserial schedule
--	transaction���� ��ó�� ����Ǵ� Schedule
--	transaction���� ��ó�� ����Ǳ� ������ ���ü��� �������� ���� �ð� ���� �� ���� transaction���� ó���� �� �ִ�.
--	transaction���� � ���·� ���ļ� ����Ǵ����� ���� �̻��� ����� �� �� �� �ִ� -> ��Ƽ������ ����ϳ׿�

--	Nonserial schedule�� �����ص� �̻��� ����� ������ ���� �� �ִ� ����� ������?
--	serial schedule�� ������ nonserial schedule�� �����ϸ� �ǰڴ�?
--	schedule�� �����ϴ�?

--	Confilct
--	of two operations
--	�� ���� ������ ��� �����ϸ� Confilct
--	1. ���� �ٸ� transaction �Ҽ�
--	2. ���� �����Ϳ� ����
--	3. �ּ� �ϳ��� write operation
--	read-write confilct
--	write-write confilct
--	������ �ٲ�� ����� �ٲ��.

--	Confilct equivalent
--	for two schedules
--	�� ���� ��� �����ϸ� confilct equivalent
--	1. �� schedule�� ���� transaction���� ������.
--	2. � confilcing operations�� ������ ���� schedule ��� �����ϴ�
--	serial schedule�� confilct equivalent�� �� -> Confict serializable -> �̻��� ���X
--	confilct serializable�� nonserial schedule�� ���
--	����
--	���� transaction�� ���ÿ� �����ص� schedule�� confilct serializable�ϵ��� �����ϴ� ���������� ����

--	concurrency control�� � schedule�� serializable�ϰ� �����
--	�̰ſ� ���� �ִ� Ʈ����� �Ӽ��� Isolation�̴�
--	�ʹ� �Ϻ��ϰ� �ҷ��� �ϸ� �����̽��� �ִ�.
--	�׷��� isolation level �� �����. ( �� �� ������ ������ �ø��� ) 

--	unrecoverable schedule
--	schedule������ commit�� transaction�� rollback�� transaction�� write �߾��� �����͸� ���� ���
--	rollback�� �ص� ���� ���·� ȸ�� �Ұ����� �� �ֱ� ������ �̷� schedule�� DBMS�� ����ϸ� �ȵȴ�.

--	recoverable schedule
--	schedule ������ �� � transaction�� �ڽ��� ���� �����͸� write�� transaction�� ���� commit/rollback �������� commit ���� �ʴ� ���

--	cascading rollback
--	�ϳ��� transaction�� rollback�ϸ� �������� �ִ� �ٸ� transaction�� rollback �ؾ��Ѵ�.
--	���� transaction�� rollback�� ���������� �Ͼ�� ó������� ���� ���.

--	cascadeless schedule ( avoid cascading rollback )
--	schedule������ � transaction�� commit ���� �ʴ� transaction���� write�� �����ʹ� ���� �ʴ� ���

--	strict schedule
--	schedule������ � transaction�� commit ���� �ʴ� transaction���� write�� �����͸� ������ ������ �ʴ� ���
--	rollback�� �� recovery�� ����. transaction ���� ���·� �������⸸ �ϸ� �ȴ�.


CREATE DATABASE designDB;

-- ��ƼƼ(���̺�)���� ���踦 ������ ���̾�׷� (ERD) -> �����ͺ��̽� ���̾�׷�

use designDB;

SELECT * FROM Company;
SELECT * FROM Department;

INSERT INTO Company VALUES(N'����');
INSERT INTO Company VALUES(N'������');
INSERT INTO Department VALUES(1,N'������',1);
INSERT INTO Department VALUES(2,N'������',1);
INSERT INTO Department VALUES(1,N'��������',2);
INSERT INTO Department (DepartmentId, DepartmentName) VALUES(2,N'��Ʈ��');

UPDATE Department SET CompanyId = 2 WHERE DepartmentId = 2;

--	�÷� �̸� ����
sp_rename 'Department.[CompnayId]', N'CompanyId', 'COLUMN'
--	�÷� Ÿ�� ����
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
--	1:M ��� ����
--	�ֻ���
INSERT INTO TestDepartment VALUES(1, '������', NULL);
INSERT INTO TestDepartment VALUES(2, '����1��', 1);
INSERT INTO TestDepartment VALUES(3, '����2��', 1);
INSERT INTO TestDepartment VALUES(4, '����1-1��', 2);
INSERT INTO TestDepartment VALUES(5, '����1-2��', 2);

SELECT * FROM TestDepartment AS A
INNER JOIN ( SELECT * FROM TestDepartment ) AS B ON A.departmentId = B.highDepartmentId
WHERE A.departmentId = 2

--  ALTER TABLE [���̺��] DROP CONSTRAINT [FOREIGN KEY��]

ALTER TABLE TestDepartment ADD CONSTRAINT Department_Company_FK FOREIGN KEY (CompanyId) REFERENCES Company(CompanyId);

-- M:N ����
-- �����Ͻ� ����?
-- �л� <-> ���� PK �ߺ� �߻�
-- �л� ������ ���谡 ���� ������ ���̺��̴�. ������ ���̺�
-- �л� ���� ���� 
-- �ϳ��� ���̺��� �ϳ��� ��ü�� �繰�� �𵨸�!
-- ���踦 ������


CREATE TABLE ���� (
	����id int PRIMARY KEY NOT NULL,
)

CREATE TABLE ���� (
	����id int PRIMARY KEY NOT NULL,
	����id int 
)

ALTER TABLE ���� ADD CONSTRAINT ����_����_FK FOREIGN KEY (����id) REFERENCES ����(����id);

CREATE TABLE å (
	åid int PRIMARY KEY NOT NULL,
)

CREATE TABLE �� (
	��id int PRIMARY KEY NOT NULL,
	���� NVARCHAR(50) NOT NULL
)

CREATE TABLE ������� (
	�������id int PRIMARY KEY NOT NULL,
)

use ��������;
 
SELECT * FROM TB_SchoolGrade;
SELECT * FROM TB_SchoolClass;

INSERT INTO TB_SchoolGrade VALUES(1, '1�г�'), (2, '2�г�'), (3, '3�г�');

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

EXEC SP_Insert_SchoolClass 3, 3, '3��'

USE designDB;

SELECT * FROM �Ŷ�
SELECT * FROM �ź�
SELECT * FROM �κ�
INSERT INTO �ź� VALUES(1, '��쵿', NULL),(2,'Ȳ����',NULL)

UPDATE �Ŷ� SET �Ŷ��� = '�ڱ浿' WHERE �Ŷ�id = 3;
UPDATE �ź� SET �Ŷ�ID = 3 WHERE �ź�id = 1;
UPDATE �Ŷ� SET �ź�ID = 1 WHERE �Ŷ�id = 3;

SELECT * FROM �Ŷ� AS A
FULL OUTER JOIN �ź� AS B ON A.�Ŷ�id = B.�Ŷ�id

INSERT INTO �κ� VALUES(1, 'ȫ�浿', '��쵿'),(2, '��浿', 'Ȳ����')

SELECT A.�Ŷ�id �Ŷ��ź�id, a.�Ŷ���, b.�źθ�
from �Ŷ�1 a join �ź�1 b on a.�Ŷ�id = b.�ź�id