use MyDatabase
go

if OBJECT_ID('Summary') is not null
	drop procedure Summary
go

create procedure Summary
as
begin
	if OBJECT_ID('SummaryTable') is not null
		drop table SummaryTable

	select 
		EmployeeId,
		sum(Amount) as 'Total',
		AVG(Amount) as 'Daily Average',
		MonthOnly into SummaryTable
	from SalesTable
	group by EmployeeId, MonthOnly
	
	if OBJECT_ID('NamesAndTotals') is not null
		drop table NamesAndTotals

	select 
		NameTable.Name, 
		NameTable.Id, 
		SummaryTable.MonthOnly,
		Total,
		[Daily Average]
		into NamesAndTotals 
		from NameTable
		inner join SummaryTable on
		NameTable.Id = SummaryTable.EmployeeId
end
