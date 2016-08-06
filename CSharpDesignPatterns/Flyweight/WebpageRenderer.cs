using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CSharpDesignPatterns.Flyweight
{
    public class WebpageRenderer
    {
        public void Render()
        {
            ImageFactory imgFactory = new ImageFactory();

            BaseImage image = imgFactory.GetFlyweight("image.png");
            image.Display(0, 0, 400, 250);

            image = imgFactory.GetFlyweight("image.png");
            image.Display(60, 420, 200, 90);

            image = imgFactory.GetFlyweight("image.png");
            image.Display(65, 925, 75, 75);
        }
    }
}
