using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Net;
using System.Runtime.Serialization;
using System.Runtime.Serialization.Json;

namespace CSharpDesignPatterns.Proxy
{
    class BitcoinAPI : CoinAPI
    {
        public override decimal GetValueInUSD()
        {
            WebRequest request = WebRequest.Create(@"http://coinabul.com/api.php");

            BitcoinResponse jsonResponse = null;
            using (HttpWebResponse response = request.GetResponse() as HttpWebResponse)
            {
                var serializer = new DataContractJsonSerializer(typeof(BitcoinResponse));
                var obj = serializer.ReadObject(response.GetResponseStream());
                jsonResponse = obj as BitcoinResponse;
            }

            return (decimal)(jsonResponse != null ? jsonResponse.BTC.USD : 0.0f);

        }
    }

    [DataContract]
    public class BitcoinResponse
    {
        [DataMember(Name = "BTC")]
        public BTCResponse BTC { get; set; }
    }

    [DataContract]
    public class BTCResponse
    {
        [DataMember(Name = "USD")]
        public float USD { get; set; }
    }
}
