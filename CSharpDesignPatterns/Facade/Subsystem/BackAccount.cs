using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CSharpDesignPatterns.Facade.Subsystem
{
    public abstract class BackAccount
    {
        private int m_iAccountNumber = 0;

        public BackAccount(int iAccountNumber)
        {
            m_iAccountNumber = iAccountNumber;
        }

        public abstract decimal GetAccountBalance();
        public abstract void Withdrawl(decimal amount);
        public abstract void Deposit(decimal amount);
    }
}
