using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Drawing;

namespace CSharpDesignPatterns.Factory_Method
{
    class Superman : Toy
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
            return Color.Blue();
        }
    }
}
