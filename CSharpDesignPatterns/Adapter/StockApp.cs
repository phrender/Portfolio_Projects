using System;
using System.Data;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CSharpDesignPatterns.Adapter
{
    class StockApp
    {
        public void ShowStockHistory(string strTicker)
        {
            StockHistoryTarget adapter = new StockHistoryAdapter();

            DataTable history = adapter.GetStockPrices(strTicker);

            foreach(DataRow row in history.Rows)
            {
                DateTime dateTime = (DateTime)row[0];
                decimal price = (decimal)row[1];
                Console.WriteLine("On {0: MMM d yyy} {1} as ${2: 0.00}", dateTime, strTicker, price);
            }
        }
    }
}
