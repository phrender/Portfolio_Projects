using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CSharpDesignPatterns.Factory_Method
{
    abstract class ToyCreator
    {
        protected abstract Toy CreateToy();

        public void MakeToy()
        {
            Toy toy = CreateToy();
            int iGrams = toy.GetPowderQuantity();
            PreparePowder(iGrams);
            SelectMold(toy);
            InjectPowder(iGrams);

            TimeSpan heatingTime = toy.GetHeatingTime();
            HeatMold(heatingTime);

            ExtractToy();

            string color = toy.GetColor();
            PaintToy(color);
        }

        #region Toy costruction methods

        private void PreparePowder(int iGrams)
        {
            Console.WriteLine("Preparing {0} grams of powder", iGrams);
        }

        private void SelectMold(Toy toy)
        {
            Console.WriteLine("Selecting mold {0}", toy.GetType().Name);
        }

        private void InjectPowder(int iGrams)
        {
            Console.WriteLine("Injecting {0} grams of powder", iGrams);
        }

        private void HeatMold(TimeSpan duration)
        {
            Console.WriteLine("Heating for {0} minutes", duration.TotalMinutes);
        }

        private void ExtractToy()
        {
            Console.WriteLine("Extracting toy");
        }

        private void PaintToy(string color)
        {
            Console.WriteLine("Painting toy {0}", color);
        }

        #endregion
    }
}
