using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CSharpBehaviouralDesignPatterns.ChainOfResponsibility
{
    class CEO : POApprover
    {
        public CEO(POApprover successor)
        {
            m_successor = successor;
        }

        public override void ProcessRequest(decimal price)
        {
            if (price <= 100000)
            {
                Console.WriteLine("${0} purchase approved by {1}", price, this.GetType().Name);
            }
            else
            {
                Console.WriteLine("Plan executive meeting to approve ${0} purchase.", price);
            }
        }
    }
}
