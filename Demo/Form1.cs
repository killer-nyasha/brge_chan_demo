using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.IO;
using System.Drawing.Imaging;

using AttachmentsProcessor;

namespace Demo
{
    public partial class Form1 : Form
    {
        BrgeAttachableImagesSet bais;
        BackgroundWorker worker;

        public Form1()
        {
            InitializeComponent();

            Load += (_s, e) =>
            {
                worker = new BackgroundWorker();
                worker.WorkerReportsProgress = true;
                BrgeAttachableImagesSet.worker = worker;
                pictureBox1.SizeMode = PictureBoxSizeMode.Zoom;

                worker.DoWork += (s, a) =>
                {
                    bais = new BrgeAttachableImagesSet(new DirectoryInfo(@"132"));
                    bais.update();
                    update();
                };

                worker.RunWorkerCompleted += (s, a) =>
                {
                    pictureBox1.Image = bais.temp;
                    //pictureBox1.Size = new Size(bais.temp.Width * 2 / 3, bais.temp.Height * 2 / 3);
                    //Point p = new Point(200, 50);
                    //pictureBox1.Location = p;
                    //Size = new Size(pictureBox1.Width + p.X, pictureBox1.Height + p.Y);
                };

                worker.ProgressChanged += (s, a) => progressBar1.Value = a.ProgressPercentage;

                worker.RunWorkerAsync();
            };

        }

        private void button1_Click(object sender, EventArgs e)
        {
            worker = new BackgroundWorker();
            worker.WorkerReportsProgress = true;
            BrgeAttachableImagesSet.worker = worker;
            worker.DoWork += (s, a) =>
            {
                bais.setShaderForTag("hair", textBox1.Text);
                bais.setShaderForTag("eye", textBox2.Text);
                //bais.setShaderForTag("contour", textBox1.Text);
                bais.setShaderForTag("body", textBox3.Text);
                bais.setShaderForTag("armor", textBox4.Text);
            };

            worker.RunWorkerCompleted += (s, a) =>
            {
                update();
            };

            worker.RunWorkerAsync();

        }

        void update()
        {
            pictureBox1.Image = bais.temp;
        }
    }

    public class BrgeAttachableImagesSet
    {
        public static BackgroundWorker worker;

        static int GetLayerMode(string fileName)
        {
            if (fileName.Contains("mul"))
                return 2;

            if (fileName.Contains("ldark"))
                return 1;

            if (fileName.Contains("contour"))
                return 1;

            return 0;
        }

        static string GetLayerTag(string fileName)
        {
            if (fileName.Contains("newhair"))
                return "hair";

            if (fileName.Contains("eye-"))
                return "eye";

            if (fileName.Contains("contour"))
                return "contour";

            if (fileName.Contains("armor_fill")
                || fileName.Contains("gloves_fill"))
                return "armor";

            if (fileName.Contains("body_fill")
                || fileName.Contains("head_fill")
                || fileName.Contains("hand1_fill")
                || fileName.Contains("hand2_fill")
                || fileName.Contains("leg1_fill")
                || fileName.Contains("leg2_fill")
                )
                return "body";

            return "";
        }

        public Bitmap temp;

        public BrgeAttachableImagesSet(DirectoryInfo dir)
        {
            FileInfo[] fi = dir.GetFiles();

            for (int i = 0; i < fi.Length; i++)
            {
                Bitmap bmp = new Bitmap(fi[i].FullName);

                if (bmp == null)
                    throw new Exception();

                if (bmp.PixelFormat != PixelFormat.Format32bppArgb)
                {
                    Bitmap newbmp = new Bitmap(bmp.Width, bmp.Height, PixelFormat.Format32bppArgb);
                    for (int y = 0; y < bmp.Height; y++)
                        for (int x = 0; x < bmp.Width; x++)
                            newbmp.SetPixel(x, y, bmp.GetPixel(x, y));
                    bmp.Dispose();
                    newbmp.Save(fi[i].FullName);
                    MessageBox.Show("converted!");
                    bmp = newbmp;
                }
                //    throw new Exception();

                if (temp == null)
                temp = new Bitmap(bmp);

                layers.Add(new Layer(bmp, GetLayerTag(fi[i].Name), GetLayerMode(fi[i].Name), 100));
            }
            layers.Reverse();

        }

        public void update()
        {
            BitmapData tempData = temp.LockBits(new Rectangle(0, 0, temp.Width, temp.Height), ImageLockMode.ReadWrite, temp.PixelFormat);

            layers[0].staticUpdate(tempData);

            for (int i = 0; i < layers.Count; i++)
            {
                layers[i].update(i == 0);
                worker.ReportProgress((int)((float)i / (layers.Count-1) * 100));
            }

            temp.UnlockBits(tempData);
        }

        public void setShaderForTag(string tag, string shader)
        {
            for (int i = 0; i < layers.Count; i++)
            {
                if (layers[i].tag == tag)
                    layers[i].setShader(shader);
            }
            update();
        }

        List<Layer> layers = new List<Layer>();
    };
}
