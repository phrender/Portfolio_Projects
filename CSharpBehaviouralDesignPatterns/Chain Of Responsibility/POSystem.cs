using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CSharpBehaviouralDesignPatterns.ChainOfResponsibility
{
    public class POSystem
    {
        protected POApprover m_approvalChain = null;

        public POSystem()
        {
            m_approvalChain = new Manager(new VicePresident(new CEO(null)));
        }

        public void ProcessRequest(decimal price)
        {
            m_approvalChain.ProcessRequest(price);
        }
    }
}
