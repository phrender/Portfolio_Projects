using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CSharpDesignPatterns.Composite
{
    public abstract class HtmlTag : HtmlNode
    {
        protected string m_strTagName = string.Empty;
        protected List<HtmlNode> m_htmlElements = new List<HtmlNode>();

        protected HtmlTag(string strTagName)
        {
            m_strTagName = strTagName;
        }

        public void AddHtmlChild(HtmlNode component)
        {
            m_htmlElements.Add(component);
        }

        public override string Render()
        {
            StringBuilder stringBuilder = new StringBuilder();
            stringBuilder.AppendFormat("<{0}>\n", m_strTagName);

            foreach(HtmlNode htmlChild in m_htmlElements)
            {
                stringBuilder.Append(htmlChild.Render());
            }

            stringBuilder.AppendFormat("</{0}>\n", m_strTagName);

            return stringBuilder.ToString();
        }
    }
}
