using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CSharpDesignPatterns.Composite
{
    public class Text : LeaftNode
    {
        protected string m_strText = string.Empty;

        public Text(string strText)
        {
            m_strText = strText;
        }

        public override string Render()
        {
            return m_strText;
        }
    }
}
