using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using System.Web.UI;
using System.Web.UI.WebControls;

namespace DashboardExample
{
    public partial class Default : System.Web.UI.Page
    {
        protected void Page_Load(object sender, EventArgs e)
        {

        }

        protected void DropDownList1_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (DropDownList1.SelectedValue == "Pie")
            {
                Chart1.Series[0].ChartType = System.Web.UI.DataVisualization.Charting.SeriesChartType.Pie;
            }
            else if (DropDownList1.SelectedValue == "Bar")
            {
                Chart1.Series[0].ChartType = System.Web.UI.DataVisualization.Charting.SeriesChartType.Bar;
            }
            else
            {
                Chart1.Series[0].ChartType = System.Web.UI.DataVisualization.Charting.SeriesChartType.Line;
            }
        }
    }
}