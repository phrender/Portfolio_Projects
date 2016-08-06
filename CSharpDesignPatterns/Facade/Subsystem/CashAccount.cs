using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CSharpDesignPatterns.Facade.Subsystem
{
    public class CashAccount : BackAccount
    {
        protected decimal m_balance = 1000m;

        public CashAccount(int iAccountNumber) : base(iAccountNumber)
        {
        }

        public override decimal GetAccountBalance()
        {
            return m_balance;
        }

        public override void Withdrawl(decimal amount)
        {
            m_balance -= amount;
        }

        public override void Deposit(decimal amount)
        {
            m_balance += amount;
        }
    }
}
