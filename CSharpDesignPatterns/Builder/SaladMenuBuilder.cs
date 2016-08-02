using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CSharpDesignPatterns.Builder
{
    class SaladMenuBuilder : MenuBuilder
    {
        private Menu m_menu = new Menu();

        public override void BuildBurgerOrSalad()
        {
            m_menu.Add("Salad");
        }

        public override void BuildFries()

        {
            m_menu.Add("Fries");
        }

        public override void BuildDessert()
        {
            m_menu.Add("Dessert");
        }

        public override void BuildDrink()
        {
            m_menu.Add("Drink");
        }

        public override void BuildToy()
        {
        }

        public override Menu GetResult()
        {
            return m_menu;
        }
    }
}
