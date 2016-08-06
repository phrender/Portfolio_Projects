using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CSharpDesignPatterns.Facade
{
    public class MutualFund
    {
        private int m_iAccountNumber = 0;

        public MutualFund(int iAccountNumber)
        {
            m_iAccountNumber = iAccountNumber;
        }
        
        public void Buy(string strTickerSymbol, int iShares)
        {
            Subsystem.CashAccount account = new Subsystem.CashAccount(m_iAccountNumber);
            Subsystem.EquityFactory equityFactory = Subsystem.EquityFactory.GetInstance();
            Subsystem.Equity equity = equityFactory.CreateEquity(strTickerSymbol);

            decimal amount = equity.GetPrice() * iShares;
            decimal balance = account.GetAccountBalance();

            if (balance >= amount)
            {
                account.Withdrawl(amount);

                equity.Buy(iShares);
                Console.WriteLine("Successfully bought {0} {1} shares.", iShares, strTickerSymbol);
                Console.WriteLine("New account balance is ${0}.", account.GetAccountBalance());
            }
            else
            {
                Console.WriteLine("Insufficient balance ${0}.", balance);
            }
        }
    }
}
