use MyDatabase
go

if OBJECT_ID('DeleteRecords') is not null
	drop procedure DeleteRecords
go

create procedure DeleteRecords
as
begin
	if OBJECT_ID('BackupSalesTable') is not null
		drop table BackupSalesTable
		select * into BackupSalesTable from SalesTable

	delete from SalesTable where EmployeeId not in (select Id from NameTable)
end
