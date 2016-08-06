using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CSharpDesignPatterns.Builder
{
    class Menu
    {

        private Dictionary<string, string> m_menuParts = new Dictionary<string, string>();

        public void Add(string part)
        {
            m_menuParts.Add(part, part);
        }

        public override string ToString()
        {
            return String.Join(", ", m_menuParts.Values);
        }

    }
}
