use MyDatabase 
go

if OBJECT_ID('SalesTable') is not null
	drop table SalesTable

if OBJECT_ID('NameTable') is not null
	drop table NameTable

create table NameTable (Id int identity(1, 1) not null primary key, Name varchar(50))

insert into NameTable(Name) values ('John')
insert into NameTable(Name) values ('Steve')

create table SalesTable (Id int identity(1, 1) not null primary key, EmployeeId int, DateSold date, MonthOnly varchar(50), Amount money)

declare @OuterCounter int = 1
declare @InnerCounter int = 1

while @OuterCounter <= (select COUNT(Name) from NameTable)
begin -- OuterCounter loop begin
	while @InnerCounter <= DATEDIFF(DAY, GETDATE() - DATEPART(DAY, GETDATE()), CONCAT('12/31/', DATEPART(YEAR, GETDATE())))
	begin -- InnerCounter loop begin
		insert into SalesTable 
				(EmployeeId, DateSold, MonthOnly, Amount) 
			values 
				(@OuterCounter, GETDATE() - DATEPART(DAY, GETDATE()) + @InnerCounter, DATENAME(MONTH, GETDATE() - DATEPART(DAY, GETDATE()) + @InnerCounter), RAND() * 10000)
		set @InnerCounter = @InnerCounter + 1
	end	-- InnerCounter loop end
	set @OuterCounter = @OuterCounter + 1
	set @InnerCounter = 1
end -- OuterCounter loop end

select * from NameTable
select * from SalesTable
select * from BackupSalesTable where EmployeeId=3

go
if OBJECT_ID('UpdateNameGetNewRecordsAppend') is not null
	drop trigger UpdateNameGetNewRecordsAppend
go

create trigger UpdateNameGetNewRecordsAppend
on dbo.NameTable
after insert
	as
		set identity_insert dbo.SalesTable on
		execute dbo.AddRecords
		execute dbo.Summary
go

if OBJECT_ID('DeleteRecordsTrigger') is not null
	drop trigger DeleteRecordsTrigger
go

create trigger DeleteRecordsTrigger
on dbo.NameTable
after delete
as
	execute DeleteRecords
go