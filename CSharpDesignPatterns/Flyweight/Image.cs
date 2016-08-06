using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CSharpDesignPatterns.Flyweight
{
    class Image : BaseImage
    {
        protected string m_strFilename = string.Empty;

        public Image(string strFilename)
        {
            m_strFilename = strFilename;
        }

        public override void Display(int iX, int iY, int iWidth, int iHeight)
        {
            Console.WriteLine("@<src={0} style= left:{1}px; top:{2}px; wiidth:{3}px; height:{4}px;/>", m_strFilename, iX, iY, iWidth, iHeight);
        }
    }
}
