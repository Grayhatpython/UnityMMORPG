CREATE DATABASE GameServerDB;

USE GameServerDB;

SET NOCOUNT ON
GO

IF EXISTS (	SELECT * FROM INFORMATION_SCHEMA.TABLES 
			WHERE TABLE_NAME = 'Player' AND TABLE_SCHEMA = 'dbo' )
	DROP TABLE Player;
GO

CREATE TABLE Player (
	playerDbId		INT			NOT NULL PRIMARY KEY IDENTITY(1,1),
	accountDbId		BIGINT		NOT NULL,
	createDate		DATETIME	NOT NULL DEFAULT GETDATE()
);
GO

CREATE TABLE Item (
	itemDbId		BIGINT		NOT NULL PRIMARY KEY IDENTITY(1,1),
	accountDbId		BIGINT		NOT NULL,
	templateId		INT			NOT NULL,
	equipSlot		INT			NOT NULL,
	count			INT			NOT NULL,
	ownerDbId		INT			NOT NULL,
	enchantCount	INT			NOT NULL,
);
GO

IF EXISTS ( SELECT * FROM sys.objects 
			WHERE type = 'P' AND OBJECT_ID = OBJECT_ID('usp_CreatePlayer'))
	DROP PROCEDURE usp_CreatePlayer
GO

CREATE PROCEDURE usp_CreatePlayer
    @accountId BIGINT
AS
BEGIN
    SET NOCOUNT ON;

    INSERT INTO Player (accountDbId)  
    VALUES (@accountId);

    SELECT SCOPE_IDENTITY();  -- 현재 세션에서 생성된 ID 반환
END
GO

SELECT name, type_desc
FROM sys.procedures
WHERE name = 'usp_CreatePlayer';

SELECT SCHEMA_NAME(schema_id) AS SchemaName, name 
FROM sys.procedures
WHERE name = 'usp_CreatePlayer';

SELECT DB_NAME();

DBCC CHECKIDENT(Player, reseed, 0);
GO

EXEC usp_CreatePlayer 50
GO

SELECT * FROM PLAYER
DELETE FROM PLAYER