using NUnit.Framework;
using System;
using System.Collections.Generic;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ZBarImg.Test.Utils;
using ZXing;
using ZXing.OneD;

namespace ZBarImg.Test
{
    public class SamplesGen
    {
        [Test]
        [Ignore("Only initial run")]
        public void GenCode39()
        {
            var writer = new Code39Writer();
            foreach (var extended in new bool[] { true, false })
            {
                var reader = new BarcodeReader();
                reader.Options = new ZXing.Common.DecodingOptions
                {
                    PossibleFormats = new BarcodeFormat[] {
                    BarcodeFormat.CODE_39,
                },
                    UseCode39ExtendedMode = extended,
                    AssumeCode39CheckDigit = false,
                };
                foreach (var c in Enumerable.Range(0, 128).Select(it => (char)it))
                {
                    var target = "" + c;
                    if (extended)
                    {
                        switch (c)
                        {
                            case '$': target = "/D"; break;
                            case '%': target = "/E"; break;
                            case '+': target = "/K"; break;
                            case '/': target = "/O"; break;
                        }
                    }

                    var bars = writer.encode(target);

                    using (var bitmap = CreateBitmapFromBarcode(bars))
                    {
                        var result = reader.Decode(bitmap);
                        SaveBarCode(bitmap, $"Samples/CODE-39/Extended{(extended ? 'Y' : 'N')}/{BinUtils.FullyUrlEncode(result.Text)}.png");
                    }
                }
            }
        }

        private Bitmap CreateBitmapFromBarcode(bool[] bars)
        {
            var bitmap = new Bitmap(10 + bars.Length + 10, 10);
            using (var canvas = Graphics.FromImage(bitmap))
            {
                canvas.Clear(Color.White);

                for (int x = 0; x < bars.Length; x++)
                {
                    if (bars[x])
                    {
                        canvas.DrawLine(Pens.Black, 10 + x, 0, 10 + x, 10);
                    }
                }

                return bitmap;
            }
        }

        private void SaveBarCode(Bitmap bitmap, string filePath)
        {
            filePath = Path.GetFullPath(filePath);
            Directory.CreateDirectory(Path.GetDirectoryName(filePath));
            bitmap.Save(filePath);
        }
    }
}
