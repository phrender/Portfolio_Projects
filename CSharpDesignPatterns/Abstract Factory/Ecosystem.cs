using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CSharpDesignPatterns.Abstract_Factory
{
    class Ecosystem
    {
        private Herbivore m_herbivore;
        private Carnivore m_carbivore;

        public Ecosystem(ContinentFactory factory)
        {
            m_herbivore = factory.CreateHerbivore();
            m_carbivore = factory.CreateCarnivore();
        }

        public void Run()
        {
            m_carbivore.Eat(m_herbivore);
        }
    }
}
