using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CSharpDesignPatterns.Bridge
{
    public class OpenGLAPI : DrawAPI
    {
        public override void DrawLine(int iX1, int iY1, int iX2, int iY2)
        {
            Console.WriteLine(@"    glBegin(GL_LINES)");
            Console.WriteLine(@"    glVertex2i({0}, {1})", iX1, iY1);
            Console.WriteLine(@"    glVertex2i({0}, {1})", iX2, iY2);
            Console.WriteLine(@"    glEnd()");
        }
    }
}
