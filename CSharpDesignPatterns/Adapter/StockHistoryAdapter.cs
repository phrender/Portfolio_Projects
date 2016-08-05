using System;
using System.Collections.Generic;
using System.Data;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CSharpDesignPatterns.Adapter
{
    public class StockHistoryAdapter : StockHistoryTarget
    {
        private StockHistoryDB m_adaptee = new StockHistoryDB();

        public override DataTable GetStockPrices(string strTicker)
        {
            decimal[] history = new decimal[] { };

            switch (strTicker)
            {
                case "APPL":
                    history = m_adaptee.GetAAPL();
                    break;

                case "MSFT":
                    history = m_adaptee.GetMSFT();
                    break;

                case "GOOG":
                    history = m_adaptee.GetGOOG();
                    break;

                default:
                    throw new NotImplementedException("Failed to get history for ticker: " + strTicker);
            }

            DataTable results = new DataTable();
            results.Columns.Add(new DataColumn("Date", typeof(DateTime)));
            results.Columns.Add(new DataColumn("Price", typeof(decimal)));

            DateTime dateTime = new DateTime(DateTime.Today.Year, DateTime.Today.Month, DateTime.Today.Day);
            for (int i = 0; i < 12; i++)
            {
                DataRow row = results.NewRow();
                row[0] = dateTime;
                row[1] = history[i];
                results.Rows.Add(row);
                dateTime = dateTime.AddMonths(-1);
            }

            return results;

        }
    }
}
