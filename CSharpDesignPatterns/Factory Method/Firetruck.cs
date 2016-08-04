using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CSharpDesignPatterns.Factory_Method
{
    class Firetruck : Toy
    {
        
        public override int GetPowderQuantity()
        {
            return 40;
        }

        public override TimeSpan GetHeatingTime()
        {
            return TimeSpan.FromMinutes(2);
        }

        public override string GetColor()
        {
            return Color.Red();
        }
    }
}
