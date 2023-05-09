/* --COPYRIGHT--,BSD
 * Copyright (c) 2014, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR#
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * --/COPYRIGHT--*/
package com.ti.msp430.imagereformer;

import java.awt.image.*;
import java.io.*;
import java.sql.Date;
import java.text.SimpleDateFormat;
import java.util.Calendar;

import javax.swing.JFrame;
import javax.swing.JOptionPane;



public class Output {
	BufferedImage destination;
	String imageName;
	int compressionLevel;
	static String year = new SimpleDateFormat("yyyy").format(Calendar.getInstance().getTime());
	JFrame jf;
	
	static String license="/* Copyright (c) "+year+", Texas Instruments Incorporated\n"+
	"All rights reserved.\n"+
	"\n"+
	"Redistribution and use in source and binary forms, with or without\n"+
	"modification, are permitted provided that the following conditions\n"+
	"are met:\n"+
	"\n"+
	"*  Redistributions of source code must retain the above copyright\n"+
	"   notice, this list of conditions and the following disclaimer.\n"+
	"\n"+
	"*  Redistributions in binary form must reproduce the above copyright\n"+
	"   notice, this list of conditions and the following disclaimer in the\n"+
	"   documentation and/or other materials provided with the distribution.\n"+
	"\n"+
	"*  Neither the name of Texas Instruments Incorporated nor the names of\n"+
	"   its contributors may be used to endorse or promote products derived\n"+
	"   from this software without specific prior written permission.\n"+
	"\n"+
	"THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS \"AS IS\"\n"+
	"AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,\n"+
	"THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR\n"+
	"PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR\n"+
	"CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,\n"+
	"EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,\n"+
	"PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;\n"+
	"OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,\n"+
	"WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR\n"+
	"OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,\n"+
	"EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.*/\n";

	
	public static int UNCOMPRESSED = 0;					//original value =0
	public static int COMPRESSED = 1; // 1bpp-4bpp		//original value =1;
	public static int STANDARD_PALLET = 0;					//original value =0
	public static int ADAPTIVE_PALLET = 1; // 1bpp-4bpp		//original value =1;
	public static int COMPRESSED_RLE8_SPEED = 2;		//original value =2;
	public static int COMPRESSED_RLE8_SIZE = 3;			//original value =3;
	
	/**
	 * Simple class to overide the input file name to have no periods in it.
	 * 
	 * @author a0194920
	 * 
	 */
	class MyFile extends File {
		public MyFile(String name) {
			super(name);
		}

		public String getName() {
			String s = super.getName();
			s = s.replaceAll("\\.", "_");

			return (s);
		}
	}

	public Output(BufferedImage bi, String name, int compression,JFrame jf) {
		this.compressionLevel = compression;
		this.destination = bi;
		this.jf=jf;
		
		if (name==null) return;
		File f=new File(name);
		
		name=f.getName();
		int idx=name.indexOf(".");
		if (idx>-1) {
		   this.imageName = name.substring(0,idx);
		} else {
			//Ok no extension so just punt and use the whole string
			this.imageName = name;
		}
	}

	public void writePixels(BufferedWriter output) throws IOException {

		

		output.write("static const uint8_t pixel_" + imageName
				+ getEnc() + "[] =\n");
		output.write("{\n");

		DataBuffer buff = destination.getRaster().getDataBuffer();
		int width = destination.getWidth();
		int size = buff.getSize();
		int count = 0;

		// Get number of bits per pixels to determine which packaging scheme
		// will be applied
		int bitsPerPixel = destination.getColorModel().getPixelSize();
		int pixInByte = 0;

		if (bitsPerPixel == 1) {
			pixInByte = 8;
		} else if (bitsPerPixel == 2) {
			pixInByte = 4;
		} else if (bitsPerPixel == 4) {
			pixInByte = 2;
		} else if (bitsPerPixel == 8) {
			pixInByte = 1;
		}


		
		// If user wants to generate a compressed image
		if (compressionLevel>0) {			

			Compression compression = new Compression(destination);
			
			// If format selected is 1bpp implement rle_4_1bpp method
			if (bitsPerPixel == 1) {
				
				//Compress RLE4 method to compress 1BPP image
				DataBufferInt compBuffer = compression.rle_4_1bpp();

				//Format bytes as expected by GRLib
				for (int i = 0; i < compBuffer.getSize(); i++) {
					
					output.write("0x" + formatByte((byte) compBuffer.getElem(i)) + ", ");

					if (count == width-1) {
						count = 0;
						output.write("\n");
					}else{
						count++;
					}
				}
				output.write("\n};\n\n");
			}
			// If format selected is 4bpp implement rle_4_2bpp method
			if (bitsPerPixel == 2) {	

				
				DataBufferInt compBuffer = compression.rle_4_2bpp();

				//Format bytes as expected by GRLib
				for (int i = 0; i < compBuffer.getSize(); i++) {
					
					output.write("0x" + formatByte((byte) compBuffer.getElem(i)) + ", ");

					if (count == width-1) {
						count = 0;
						output.write("\n");
					}else{
						count++;
					}
				}
				output.write("\n};\n\n");
				
			}
			// If format selected is 4bpp implement rle_4_4bpp method
			if (bitsPerPixel == 4) {	
				
				DataBufferInt compBuffer = compression.rle_4_4bpp();

				//Format bytes as expected by GRLib
				for (int i = 0; i < compBuffer.getSize(); i++) {
					
					output.write("0x" + formatByte((byte) compBuffer.getElem(i)) + ", ");

					if (count == width-1) {
						count = 0;
						output.write("\n");
					}else{
						count++;
					}
				}
				output.write("\n};\n\n");
				
			} 
			if ((bitsPerPixel == 8) ) {
				
				//For debug purposes
				//System.out.println("Entered Compress RLE8  section");
				
				DataBufferInt compBuffer = compression.rle_8_8bpp();

				for (int i = 0; i < compBuffer.getSize(); i++) {

					output.write("0x"
							+ formatByte((byte) compBuffer.getElem(i)) + ", ");

					if (count == width-1) {
						count = 0;
						output.write("\n");
					}else{
						count++;
					}
				}
				output.write("\n};\n\n");
			} 
			
		} else  {
			
			//For debug purposes
			//System.out.println("Entered UNCOMPRESSED section");
			
			// Create an array that will hold the bytes to be formated
			byte b[] = new byte[pixInByte];

			int i = 0, bitPosition;			
			
			// Grab next bunch of bytes based on numBytes been used
			while (i + pixInByte -1 < (size - 1)) {
				
				for (int j = 0; j < pixInByte; j++) {
					count++;
					if (count <=width) {
					   b[j] = (byte) buff.getElem(i++);
					} else {
						// this means the size of the raster line is not an even
						// multiple of the number of bits/pixel
						// 1bpp width % 8 should be 0 or it will pad the last byte
						// 2bpp width % 4 should be 0 or it will pad the last byte
						// 4bpp if width is odd it will pad the last byte						
						b[j] = (byte) 0;
					}
				}

				// Output formatted bytes
				output.write("0x" + formatBytes(b, pixInByte) + ", ");
				
				if (count >= width) {
					count = 0;
					output.write("\n");
				} 
				
			}
			
			//For debug purposes
			//System.out.println("Exit while loop with i=" + i+ " and size=" + size);

			// Check if any bytes are missing to be formated
			if (i <= size - 1) {
				bitPosition = 0;

				// missing hold the number of bytes that are missing to be formated
				int missing = size - i;
				
				//For debug purposes
				//System.out.println("Entered missing byte to format loop, missing=" + missing);
				
				while (missing != 0) {
					b[bitPosition] = (byte) buff.getElem(i++);
					bitPosition++;
					missing--;
				}
				
				//For debug purposes
				//System.out.println("Exit missing byte loop missing=" + missing);
				output.write("0x" + formatBytes(b, pixInByte) + "\n};\n\n");
			} else {
				output.write("\n};\n\n");
			}
			
			//For debug purposes
			//System.out.println("Exiting UNCOMPRESSED section");

		}

	}

	
	public String convertImageSize(int intSize) {
		StringBuffer sb = new StringBuffer();
		sb.append(Integer.toString(intSize));
		sb.append(",\n");
		return (sb.toString());
	}

	public void writeEpi(BufferedWriter output) throws IOException {

		//MyFile f = new MyFile(imageName);
		String width = convertImageSize(destination.getWidth());
		String height = convertImageSize(destination.getHeight());
        
		output.write("const Graphics_Image  " + imageName + getEnc() + "=\n{\n");
		output.write("\tIMAGE_FMT_" + getEnc() + ",\n");
		output.write("\t" + width);
		output.write("\t" + height);
		if (destination.getColorModel().getPixelSize() == 1)
			output.write("\t2,\n");
		else if (destination.getColorModel().getPixelSize() == 2)
			output.write("\t4,\n");
		else if (destination.getColorModel().getPixelSize() == 4)
			output.write("\t16,\n");
		else if (destination.getColorModel().getPixelSize() == 8)
			output.write("\t256,\n");
		output.write("\tpalette_" + imageName + getEnc() + ",\n");
		output.write("\tpixel_" + imageName + getEnc() + ",\n");
		output.write("};\n\n");

	}

	
	public void writeHeader(BufferedWriter output) throws IOException {


		output.write(license+"\n\n");
		output.write("#include <ti/grlib/grlib.h>\n");
		output.write("#include <stdint.h>\n\n");
	}
	
	public void write(String file) {
		MyFile out; // declare a MyFile output object
		BufferedWriter output; // declare a print stream object

		try {
			out = new MyFile(file);
			output = new BufferedWriter(new FileWriter(out));
			writeHeader(output);
			writePixels(output);
			writePalette(output);
			writeEpi(output);
			output.close();
		} catch (Exception e) {
			System.err.println("Error writing to file");
			e.printStackTrace();
		}

	}

	private String getEnc() {
		int numPix = destination.getColorModel().getPixelSize();
		return (getCompression(numPix));
	}

	private String getCompression(int numPix) {

		if ((numPix == 1) && (compressionLevel==UNCOMPRESSED )) {
			return ("1BPP_UNCOMP");
		} else if (numPix == 4 && (compressionLevel==UNCOMPRESSED)) {
			return ("4BPP_UNCOMP");
		} else if (numPix == 2 && (compressionLevel==UNCOMPRESSED)) {
			return ("2BPP_UNCOMP");
		} else if (numPix == 8 && (compressionLevel==UNCOMPRESSED)) {
			return ("8BPP_UNCOMP");
		} else if((numPix == 1) && (compressionLevel==COMPRESSED)){
			return ("1BPP_COMP_RLE4");
		}else if((numPix == 2) && (compressionLevel==COMPRESSED)){
			return ("2BPP_COMP_RLE4");
		} else if((numPix == 4) && (compressionLevel==COMPRESSED)){
			return ("4BPP_COMP_RLE4");
		} else if ((numPix == 8) ){
			return ("8BPP_COMP_RLE8");
		}else {
			JOptionPane.showMessageDialog(jf, "Compression is not supported for that format.");		
			return ("-1");
			
		}

	}

	private String formatByte(byte b) {
		String value = Integer.toHexString(0xFF & b);
		if (value.length() < 2)
			value = "0" + value;
		return (value);
	}

	private String formatBytes(byte b[], int numBytes) {

		String value = "";
		int tempValue = 0;
		int shift = 0;

		if (numBytes == 8) {
			shift = 1;
		} else if (numBytes == 4) {
			shift = 2;
		} else if (numBytes == 2) {
			shift = 4;
		} else if (numBytes == 1) {
			shift = 0;
		}

		for (int i = 0; i < numBytes; i++) {
			tempValue = (tempValue << (shift)) + (0xFF & b[i]);
		}

		value = Integer.toHexString(0xFF & tempValue);
		if (value.length() < 2)
			value = "0" + value;

		return (value);
	}

	public void writePalette(BufferedWriter output) throws IOException {
		IndexColorModel cm = (IndexColorModel) destination.getColorModel();
		int size = cm.getMapSize();
		MyFile f = new MyFile(imageName);

		output.write("static const uint32_t palette_" + f.getName()
				+ getEnc() + "[]=\n");
		output.write("{\n");

		byte r[] = new byte[size];
		byte b[] = new byte[size];
		byte g[] = new byte[size];
		cm.getReds(r);
		cm.getBlues(b);
		cm.getGreens(g);
		for (int i = 0; i < size; i++) {
			String ir = formatByte(r[i]);
			String ig = formatByte(g[i]);
			String ib = formatByte(b[i]);
			output.write("\t0x" + ir + ig + ib);
			if ((i + 1) < size) {
				output.write(", ");
				if ((i + 1) % 4 == 0) {
					output.write("\n");
				}
			} else
				output.write("\n};\n\n");
		}

	}
}
