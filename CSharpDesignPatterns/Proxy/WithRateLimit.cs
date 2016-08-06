using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CSharpDesignPatterns.Proxy
{
    class WithRateLimit : CoinAPI
    {
        private CoinAPI m_realSubject;
        DateTime m_lastCalled = DateTime.MinValue;

        public WithRateLimit(CoinAPI realSubject)
        {
            m_realSubject = realSubject;
        }

        public override decimal GetValueInUSD()
        {
            if (DateTime.Now - m_lastCalled < TimeSpan.FromSeconds(1))
            {
                throw new InvalidOperationException("Rate limit exceeded!");
            }
            else
            {
                decimal val = m_realSubject.GetValueInUSD();
                m_lastCalled = DateTime.Now;

                return val;
            }
        }
    }
}
