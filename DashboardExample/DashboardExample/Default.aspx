<%@ Page Language="C#" AutoEventWireup="true" CodeBehind="Default.aspx.cs" Inherits="DashboardExample.Default" %>
<%@ Register Assembly="System.Web.DataVisualization, Version=4.0.0.0, Culture=neutral, PublicKeyToken=31bf3856ad364e35" Namespace="System.Web.UI.DataVisualization.Charting" TagPrefix="asp" %>

<!DOCTYPE html>

<html>
<head runat="server">
    <title>Simple Dashboard</title>
    <link href="StyleSheet.css" rel="stylesheet" />
    <script src="scripts/jquery-3.1.0.js"></script>
    <script src="scripts/jquery-ui-1.12.0.js"></script>
    <script src="JavaScript.js"></script>
</head>
<body onunload="pageUnload()">
    <form id="form1" runat="server">
        <asp:ScriptManager ID="ScriptManager1" runat="server"></asp:ScriptManager>

        <div class="dashboard" style="background-color: azure; width: 100%; margin: 20px;">
            
            <!-- Theme selector -->
            <div class="dashboard">
                <asp:UpdatePanel ID="UpdatePanel6" runat="server">
                    <ContentTemplate>
                        <asp:DropDownList ID="themelist" runat="server" AutoPostBack="True">
                            <asp:ListItem Selected="True">Coral</asp:ListItem>
                            <asp:ListItem>Blue</asp:ListItem>
                            <asp:ListItem>Azure</asp:ListItem>
                        </asp:DropDownList>
                    </ContentTemplate>
                </asp:UpdatePanel>
            </div>

            <!-- Box 1 -->
            <div class="dashboard">
                <asp:UpdatePanel ID="UpdatePanel1" runat="server">
                    <ContentTemplate>
                        <asp:DetailsView ID="DetailsView1" runat="server" Height="50px" Width="50%" AllowPaging="True" AutoGenerateRows="False" CellPadding="4" DataKeyNames="Id" DataSourceID="SqlDataSource1" ForeColor="#333333" GridLines="None">
                            <AlternatingRowStyle BackColor="White" ForeColor="#284775" />
                            <CommandRowStyle BackColor="#E2DED6" Font-Bold="True" />
                            <EditRowStyle BackColor="#999999" />
                            <FieldHeaderStyle BackColor="#E9ECF1" Font-Bold="True" />
                            <Fields>
                                <asp:BoundField DataField="Id" HeaderText="Id" InsertVisible="False" ReadOnly="True" SortExpression="Id" />
                                <asp:BoundField DataField="Name" HeaderText="Name" SortExpression="Name" />
                                <asp:CommandField ShowDeleteButton="True" ShowEditButton="True" ShowInsertButton="True" />
                            </Fields>
                            <FooterStyle BackColor="#5D7B9D" Font-Bold="True" ForeColor="White" />
                            <HeaderStyle BackColor="#5D7B9D" Font-Bold="True" ForeColor="White" />
                            <PagerStyle BackColor="#284775" ForeColor="White" HorizontalAlign="Center" />
                            <RowStyle BackColor="#F7F6F3" ForeColor="#333333" />
                        </asp:DetailsView>
                        <asp:SqlDataSource ID="SqlDataSource1" runat="server" ConnectionString="<%$ ConnectionStrings:MyDatabaseConnectionString %>" DeleteCommand="DELETE FROM [NameTable] WHERE [Id] = @Id" InsertCommand="INSERT INTO [NameTable] ([Name]) VALUES (@Name)" SelectCommand="SELECT [Id], [Name] FROM [NameTable]" UpdateCommand="UPDATE [NameTable] SET [Name] = @Name WHERE [Id] = @Id">
                            <DeleteParameters>
                                <asp:Parameter Name="Id" Type="Int32" />
                            </DeleteParameters>
                            <InsertParameters>
                                <asp:Parameter Name="Name" Type="String" />
                            </InsertParameters>
                            <UpdateParameters>
                                <asp:Parameter Name="Name" Type="String" />
                                <asp:Parameter Name="Id" Type="Int32" />
                            </UpdateParameters>
                        </asp:SqlDataSource>
                    </ContentTemplate>
                </asp:UpdatePanel>
            </div>

            <!-- Box 2 -->
            <div class="dashboard">
                <asp:UpdatePanel ID="UpdatePanel5" runat="server">
                    <ContentTemplate>
                        <asp:GridView ID="GridView1" runat="server" CellPadding="4" AllowPaging="True" AllowSorting="True" AutoGenerateColumns="False" DataKeyNames="Id" DataSourceID="SqlDataSource2" ForeColor="#333333" GridLines="None">
                            <AlternatingRowStyle BackColor="White" ForeColor="#284775" />
                            <Columns>
                                <asp:CommandField ShowSelectButton="True" />
                                <asp:BoundField DataField="Id" HeaderText="Id" SortExpression="Id" InsertVisible="False" ReadOnly="True" />
                                <asp:BoundField DataField="DateSold" HeaderText="DateSold" SortExpression="DateSold" />
                                <asp:BoundField DataField="EmployeeId" HeaderText="EmployeeId" SortExpression="EmployeeId" />
                                <asp:BoundField DataField="MonthOnly" HeaderText="MonthOnly" SortExpression="MonthOnly" />
                                <asp:BoundField DataField="Amount" HeaderText="Amount" SortExpression="Amount" />
                            </Columns>
                            <EditRowStyle BackColor="#999999" />
                            <FooterStyle BackColor="#5D7B9D" ForeColor="White" Font-Bold="True" />
                            <HeaderStyle BackColor="#5D7B9D" Font-Bold="True" ForeColor="White" />
                            <PagerStyle BackColor="#284775" ForeColor="White" HorizontalAlign="Center" />
                            <RowStyle ForeColor="#333333" BackColor="#F7F6F3" />
                            <SelectedRowStyle BackColor="#E2DED6" Font-Bold="True" ForeColor="#333333" />
                            <SortedAscendingCellStyle BackColor="#E9E7E2" />
                            <SortedAscendingHeaderStyle BackColor="#506C8C" />
                            <SortedDescendingCellStyle BackColor="#FFFDF8" />
                            <SortedDescendingHeaderStyle BackColor="#6F8DAE" />
                        </asp:GridView>
                        <asp:SqlDataSource ID="SqlDataSource2" runat="server" ConnectionString="<%$ ConnectionStrings:MyDatabaseConnectionString %>" SelectCommand="SELECT [Id], [DateSold], [EmployeeId], [MonthOnly], [Amount] FROM [SalesTable] WHERE ([EmployeeId] = @EmployeeId)">
                            <SelectParameters>
                                <asp:ControlParameter ControlID="DetailsView1" Name="EmployeeId" PropertyName="SelectedValue" Type="Int32" />
                            </SelectParameters>
                        </asp:SqlDataSource>
                    </ContentTemplate>
                </asp:UpdatePanel>
            </div>

            <!-- Box 2.1 -->
            <div id="insidediv">
                <asp:UpdatePanel ID="UpdatePanel2" runat="server">
                    <ContentTemplate>
                        <asp:DetailsView ID="DetailsView2" runat="server" Height="50px" Width="100%" CellPadding="4" ForeColor="#333333" GridLines="None" AllowPaging="True" AutoGenerateRows="False" DataKeyNames="Id" DataSourceID="SqlDataSource4">
                            <AlternatingRowStyle BackColor="White" ForeColor="#284775" />
                            <CommandRowStyle BackColor="#E2DED6" Font-Bold="True" />
                            <EditRowStyle BackColor="#999999" />
                            <FieldHeaderStyle BackColor="#E9ECF1" Font-Bold="True" />
                            <Fields>
                                <asp:BoundField DataField="Id" HeaderText="Id" InsertVisible="False" ReadOnly="True" SortExpression="Id" />
                                <asp:BoundField DataField="EmployeeId" HeaderText="EmployeeId" SortExpression="EmployeeId" />
                                <asp:BoundField DataField="MonthOnly" HeaderText="MonthOnly" SortExpression="MonthOnly" />
                                <asp:BoundField DataField="DateSold" HeaderText="DateSold" SortExpression="DateSold" />
                                <asp:BoundField DataField="Amount" HeaderText="Amount" SortExpression="Amount" />
                            </Fields>
                            <FooterStyle BackColor="#5D7B9D" Font-Bold="True" ForeColor="White" />
                            <HeaderStyle BackColor="#5D7B9D" Font-Bold="True" ForeColor="White" />
                            <PagerStyle BackColor="#284775" ForeColor="White" HorizontalAlign="Center" />
                            <RowStyle BackColor="#F7F6F3" ForeColor="#333333" />
                        </asp:DetailsView>
                        <asp:SqlDataSource ID="SqlDataSource4" runat="server" ConflictDetection="CompareAllValues" ConnectionString="<%$ ConnectionStrings:MyDatabaseConnectionString %>" DeleteCommand="DELETE FROM [SalesTable] WHERE [Id] = @original_Id AND (([EmployeeId] = @original_EmployeeId) OR ([EmployeeId] IS NULL AND @original_EmployeeId IS NULL)) AND (([MonthOnly] = @original_MonthOnly) OR ([MonthOnly] IS NULL AND @original_MonthOnly IS NULL)) AND (([DateSold] = @original_DateSold) OR ([DateSold] IS NULL AND @original_DateSold IS NULL)) AND (([Amount] = @original_Amount) OR ([Amount] IS NULL AND @original_Amount IS NULL))" InsertCommand="INSERT INTO [SalesTable] ([EmployeeId], [MonthOnly], [DateSold], [Amount]) VALUES (@EmployeeId, @MonthOnly, @DateSold, @Amount)" OldValuesParameterFormatString="original_{0}" SelectCommand="SELECT [Id], [EmployeeId], [MonthOnly], [DateSold], [Amount] FROM [SalesTable] WHERE ([Id] = @Id)" UpdateCommand="UPDATE [SalesTable] SET [EmployeeId] = @EmployeeId, [MonthOnly] = @MonthOnly, [DateSold] = @DateSold, [Amount] = @Amount WHERE [Id] = @original_Id AND (([EmployeeId] = @original_EmployeeId) OR ([EmployeeId] IS NULL AND @original_EmployeeId IS NULL)) AND (([MonthOnly] = @original_MonthOnly) OR ([MonthOnly] IS NULL AND @original_MonthOnly IS NULL)) AND (([DateSold] = @original_DateSold) OR ([DateSold] IS NULL AND @original_DateSold IS NULL)) AND (([Amount] = @original_Amount) OR ([Amount] IS NULL AND @original_Amount IS NULL))">
                            <DeleteParameters>
                                <asp:Parameter Name="original_Id" Type="Int32" />
                                <asp:Parameter Name="original_EmployeeId" Type="Int32" />
                                <asp:Parameter Name="original_MonthOnly" Type="String" />
                                <asp:Parameter DbType="Date" Name="original_DateSold" />
                                <asp:Parameter Name="original_Amount" Type="Decimal" />
                            </DeleteParameters>
                            <InsertParameters>
                                <asp:Parameter Name="EmployeeId" Type="Int32" />
                                <asp:Parameter Name="MonthOnly" Type="String" />
                                <asp:Parameter DbType="Date" Name="DateSold" />
                                <asp:Parameter Name="Amount" Type="Decimal" />
                            </InsertParameters>
                            <SelectParameters>
                                <asp:ControlParameter ControlID="GridView1" Name="Id" PropertyName="SelectedValue" Type="Int32" />
                            </SelectParameters>
                            <UpdateParameters>
                                <asp:Parameter Name="EmployeeId" Type="Int32" />
                                <asp:Parameter Name="MonthOnly" Type="String" />
                                <asp:Parameter DbType="Date" Name="DateSold" />
                                <asp:Parameter Name="Amount" Type="Decimal" />
                                <asp:Parameter Name="original_Id" Type="Int32" />
                                <asp:Parameter Name="original_EmployeeId" Type="Int32" />
                                <asp:Parameter Name="original_MonthOnly" Type="String" />
                                <asp:Parameter DbType="Date" Name="original_DateSold" />
                                <asp:Parameter Name="original_Amount" Type="Decimal" />
                            </UpdateParameters>
                        </asp:SqlDataSource>
                    </ContentTemplate>
                </asp:UpdatePanel>
            </div>

            <!-- Box 3 -->
            <div class="dashboard">
                <asp:UpdatePanel ID="UpdatePanel3" runat="server">
                    <ContentTemplate>
                        <asp:GridView ID="GridView2" runat="server" AllowPaging="True" AllowSorting="True" AutoGenerateColumns="False" CellPadding="4" DataSourceID="SqlDataSource3" ForeColor="#333333" GridLines="None">
                            <AlternatingRowStyle BackColor="White" ForeColor="#284775" />
                            <Columns>
                                <asp:BoundField DataField="Name" HeaderText="Name" SortExpression="Name" />
                                <asp:BoundField DataField="Id" HeaderText="Id" SortExpression="Id" />
                                <asp:BoundField DataField="MonthOnly" HeaderText="MonthOnly" SortExpression="MonthOnly" />
                                <asp:BoundField DataField="Total" HeaderText="Total" SortExpression="Total" />
                                <asp:BoundField DataField="Daily_Average" HeaderText="Daily_Average" SortExpression="Daily_Average" />
                            </Columns>
                            <EditRowStyle BackColor="#999999" />
                            <FooterStyle BackColor="#5D7B9D" Font-Bold="True" ForeColor="White" />
                            <HeaderStyle BackColor="#5D7B9D" Font-Bold="True" ForeColor="White" />
                            <PagerStyle BackColor="#284775" ForeColor="White" HorizontalAlign="Center" />
                            <RowStyle BackColor="#F7F6F3" ForeColor="#333333" />
                            <SelectedRowStyle BackColor="#E2DED6" Font-Bold="True" ForeColor="#333333" />
                            <SortedAscendingCellStyle BackColor="#E9E7E2" />
                            <SortedAscendingHeaderStyle BackColor="#506C8C" />
                            <SortedDescendingCellStyle BackColor="#FFFDF8" />
                            <SortedDescendingHeaderStyle BackColor="#6F8DAE" />
                        </asp:GridView>
                        <asp:SqlDataSource ID="SqlDataSource3" runat="server" ConnectionString="<%$ ConnectionStrings:MyDatabaseConnectionString %>" SelectCommand="SELECT [Name], [Id], [MonthOnly], [Total], [Daily Average] AS Daily_Average FROM [NamesAndTotals] WHERE ([Id] = @Id)">
                            <SelectParameters>
                                <asp:ControlParameter ControlID="DetailsView1" Name="Id" PropertyName="SelectedValue" Type="Int32" />
                            </SelectParameters>
                        </asp:SqlDataSource>
                    </ContentTemplate>
                </asp:UpdatePanel>
            </div>

            <!-- Box 4 -->
            <div class="dashboard">
            <asp:UpdatePanel ID="UpdatePanel4" runat="server">
                <ContentTemplate>
                    <asp:DropDownList ID="DropDownList1" runat="server" AutoPostBack="True" OnSelectedIndexChanged="DropDownList1_SelectedIndexChanged">
                        <asp:ListItem>Pie</asp:ListItem>
                        <asp:ListItem>Bar</asp:ListItem>
                        <asp:ListItem>Line</asp:ListItem>
                    </asp:DropDownList>
                    <br />
                    <asp:Chart ID="Chart1" runat="server" DataSourceID="SqlDataSource2">
                        <Series>
                            <asp:Series Name="Series1" ChartType="Line" XValueMember="Amount" YValueMembers="DateSold" YValuesPerPoint="2"></asp:Series>
                        </Series>
                        <ChartAreas>
                            <asp:ChartArea Name="ChartArea1"></asp:ChartArea>
                        </ChartAreas>
                    </asp:Chart>
                </ContentTemplate>
            </asp:UpdatePanel>
        </div>

        <div id="sticky" style="display: table; border: 15px solid white;">
            <textarea id="TextArea1" runat="server" style="background-color: yellow;" cols="20" rows="5" spellcheck="true"></textarea>
        </div>

    </form>
</body>
</html>
