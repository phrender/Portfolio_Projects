using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CSharpDesignPatterns.Flyweight
{
    class ImageFactory
    {

        private Dictionary<string, BaseImage> flyweights = new Dictionary<string, BaseImage>();

        public BaseImage GetFlyweight(string strFilename)
        {
            BaseImage flyweightImage = null;
            Console.WriteLine();

            if (flyweights.ContainsKey(strFilename))
            {
                flyweightImage = flyweights[strFilename] as BaseImage;
                Console.WriteLine("Returning cached image {0}", strFilename);
            }
            else
            {
                flyweightImage = new Image(strFilename);
                flyweights.Add(strFilename, flyweightImage);
                Console.WriteLine("Instantiating new image {0}", strFilename);
            }

            return flyweightImage;
        }

    }
}
