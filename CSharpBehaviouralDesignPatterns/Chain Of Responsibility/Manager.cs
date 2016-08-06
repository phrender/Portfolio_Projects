using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CSharpBehaviouralDesignPatterns.ChainOfResponsibility
{
    class Manager : POApprover
    {
        public Manager(POApprover successor)
        {
            m_successor = successor;
        }

        public override void ProcessRequest(decimal price)
        {
            if (price <= 10000)
            {
                Console.WriteLine("${0} purchase approved by {1}", price, this.GetType().Name);
            }
            else if (m_successor != null)
            {
                m_successor.ProcessRequest(price);
            }
        }
    }
}
