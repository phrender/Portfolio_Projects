using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CSharpDesignPatterns.Proxy
{
    abstract class CoinAPI
    {
        public abstract decimal GetValueInUSD();
    }
}
