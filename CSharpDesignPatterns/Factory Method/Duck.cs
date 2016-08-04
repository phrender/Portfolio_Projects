using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CSharpDesignPatterns.Factory_Method
{
    class Duck : Toy
    {
        public override int GetPowderQuantity()
        {
            return 20;
        }

        public override TimeSpan GetHeatingTime()
        {
            return TimeSpan.FromMinutes(1);
        }

        public override string GetColor()
        {
            return Color.Yellow();
        }
    }
}
