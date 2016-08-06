using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CSharpBehaviouralDesignPatterns.ChainOfResponsibility
{
    public abstract class POApprover
    {
        protected POApprover m_successor;

        public abstract void ProcessRequest(decimal price);
    }
}
