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
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * --/COPYRIGHT--*/
package com.ti.msp430.imagereformer;
/* This is a new comment*/

import java.awt.image.BufferedImage;
import java.awt.image.DataBuffer;
import java.awt.image.DataBufferInt;
import java.math.BigInteger;

public class Compression {
	    BufferedImage destination;

	    
	public Compression(BufferedImage imageBuff) {	
		this.destination=imageBuff;
	}


	public DataBufferInt rle_4_1bpp() {
		DataBuffer buff = destination.getRaster().getDataBuffer();
		DataBufferInt tempBuffer= new DataBufferInt(buff.getSize()*10);
		byte nextByte[]=new byte[8];
		
	    int encodedLength = 0;	//Store the size of compressed buffer
		int runLength = 0;		//Store size of run length
		int count = 0;			//Keeps track of byte that has been compressed in "bufferedImage";
		byte currentPixel;
		
		//Grabs the next set of 8 bytes
		for(int i=0; i < 8; i++){
			nextByte[i] = (byte) buff.getElem(count++);
		}
		
		// Set Next pixel as first pixel so that on entry to the while loop the update to pixels
		// Makes current pixel the first pixel and next pixel the second pixel.
		byte nextPixel = nextByte[0];
		int index = 1;
		int length = buff.getSize();
		
		
		while(count < length)
		{
			currentPixel = nextPixel;
			
			// If index=8 then read the next 8 bytes from buffered Image
			if(index == 8)
			{
				if(length - count > 7){
					for(int i=0; i < 8; i++)
					{
						nextByte[i] = (byte) buff.getElem(count++);
					}
				}else{
					int missing= length - count -1;
					
					for(int i=0; i <= missing; i++){
						nextByte[i]= (byte)buff.getElem(count++);
					}
					
				}
				
				nextPixel = nextByte[0];
				index = 1;
			} else {
				nextPixel = nextByte[index++]; 		// Assign nextByte to nextPixel 

			}
			
			// While in a run of the same color, and run_length of char size,
			// Increment the run length and get next color
			while((nextPixel == currentPixel) && (runLength < 15) && (count < length))
			{
				
				runLength++;
				currentPixel = nextPixel;
				if(index == 8)
				{
					if(length - count > 7){
						
						for(int i=0; i < 8; i++)
						{
							nextByte[i] = (byte) buff.getElem(count++);
						}
					}else{
						int missing= length - count -1;
						for(int i=0; i <= missing; i++){
							nextByte[i]= (byte)buff.getElem(count++);
						}
						
					}
					nextPixel = nextByte[0];
					index = 1;
				}else {
					nextPixel = nextByte[index++]; 		// Assign nextByte to nextPixel 

				}
			}
			
			tempBuffer.setElem(encodedLength++, (runLength << 4) | currentPixel);
			runLength = 0;
		}			
		
		
		DataBufferInt compBuffer = new DataBufferInt(encodedLength);
		
		for(int i=0; i < encodedLength; i++)
		{
			compBuffer.setElem(i,tempBuffer.getElem(i));
		}
			
		return(compBuffer);
	}				

	public DataBufferInt rle_4_2bpp() {
		DataBuffer buff = destination.getRaster().getDataBuffer();
		DataBufferInt tempBuffer= new DataBufferInt(buff.getSize()*10);
		byte nextByte[]=new byte[4];
		
	    int encodedLength = 0;	//Store the size of compressed buffer
		int runLength = 0;		//Store size of run length
		int count = 0;			//Keeps track of byte that has been compressed in "bufferedImage";
		byte currentPixel;
		
		//Grabs the next set of 4 bytes
		for(int i=0; i < 4; i++){
			nextByte[i] = (byte) buff.getElem(count++);
		}
		
		// Set Next pixel as first pixel so that on entry to the while loop the update to pixels
		// Makes current pixel the first pixel and next pixel the second pixel.
		byte nextPixel = nextByte[0];
		int index = 1;
		int length = buff.getSize();
		
		while(count < length)
		{
			currentPixel = nextPixel;
			
			// If index=4 then read the next 4 bytes from buffered Image
			if(index == 4)
			{
				if(length - count > 3){
					for(int i=0; i < 4; i++)
					{
						nextByte[i] = (byte) buff.getElem(count++);
					}
				}else{
					int missing= length - count -1;
					
					for(int i=0; i <= missing; i++){
						nextByte[i]= (byte)buff.getElem(count++);
					}
					
				}
				nextPixel = nextByte[0];
				index = 1;
			} else {
				nextPixel = nextByte[index++]; 		// Assign nextByte to nextPixel 
			}
			
			// While in a run of the same color, and run_length of char size,
			// Increment the run length and get next color
			while((nextPixel == currentPixel) && (runLength < 15) && (count < length))
			{
				runLength++;
				currentPixel = nextPixel;
				if(index == 4)
				{
					if(length - count > 3){
						for(int i=0; i < 3; i++)
						{
							nextByte[i] = (byte) buff.getElem(count++);
						}
					}else{
						int missing= length - count -1;
						
						for(int i=0; i <= missing; i++){
							nextByte[i]= (byte)buff.getElem(count++);
						}
						
					}
					nextPixel = nextByte[0];
					index = 1;
				}else {
					nextPixel = nextByte[index++]; 		// Assign nextByte to nextPixel 
				}
			}
			tempBuffer.setElem(encodedLength++, (runLength << 4) | currentPixel);
			runLength = 0;
		}
		
		
		DataBufferInt compBuffer = new DataBufferInt(encodedLength);
		
		for(int i=0; i<encodedLength; i++)
		{
			compBuffer.setElem(i,tempBuffer.getElem(i));
		}
		
		
		return(compBuffer);
	}
	
	
	public DataBufferInt rle_4_4bpp() {
			
		DataBuffer buff = destination.getRaster().getDataBuffer();
		DataBufferInt tempBuffer= new DataBufferInt(buff.getSize());
		
	    int encodedLength = 0;	//Store the size of compressed buffer
		int runLength = 0;		//Store size of run length
		int count = 0;			//Keeps track of byte that has been compressed in "bufferedImage";
		byte currentPixel;		//Keeps track of current pixel being RLE encoded
		
		byte nextByte[]= new byte[2];	
		nextByte[0] = (byte) buff.getElem(count++);	// High byte
		nextByte[1] = (byte) buff.getElem(count++);	// Low byte
		
		// Set Next pixel as first pixel so that on entry to the while loop the update to pixels
		// Makes current pixel the first pixel and next pixel the second pixel. (Requires ulIndex = 0)
		byte nextPixel = nextByte[0];
		int index = 1;
		int length = buff.getSize();
		
		while(count < length)
		{
			currentPixel = nextPixel;
			if(index == 1)
			{
				index = 0;
				if(length - count > 0){
					nextByte[0] = (byte) buff.getElem(count++);	// High byte
					nextByte[1] = (byte) buff.getElem(count++);	// Low byte
				}else{
					nextByte[0] = (byte) buff.getElem(count++);	// High byte
				}
				nextPixel = nextByte[0];
				
			}
			else
			{
				nextPixel = nextByte[1];
				index = 1;
			}
			
			// While in a run of the same color, and run_length of char size,
			// Increment the run length and increment the data pointer to the next color
			while((nextPixel == currentPixel) && (runLength < 15) && (count < length))
			{
				runLength++;
				currentPixel = nextPixel;
				if(index == 1)
				{
					index = 0;
					if(length - count > 0){
						nextByte[0] = (byte) buff.getElem(count++);	// High byte
						nextByte[1] = (byte) buff.getElem(count++);	// Low byte
					}else{
						nextByte[0] = (byte) buff.getElem(count++);	// High byte
					}
					nextPixel = nextByte[0];
				}
				else
				{
					index = 1;
					nextPixel = nextByte[1];
				}
			}
			tempBuffer.setElem(encodedLength++, (runLength << 4) | currentPixel);
			runLength = 0;
					
		}
		
		DataBufferInt compBuffer = new DataBufferInt(encodedLength);
		
		for(int i=0; i<encodedLength; i++)
		{
			compBuffer.setElem(i,tempBuffer.getElem(i));
		}
		
		
		return(compBuffer);
	}
		
	
	public DataBufferInt rle_8_8bpp() {
		DataBuffer buff = destination.getRaster().getDataBuffer();
		DataBufferInt tempBuffer= new DataBufferInt(buff.getSize()*3);
		int count=0;
		int length = buff.getSize();
		int runLength = 0;		//Store size of run length
		int encodedLength = 0;	//Store the size of compressed buffer
		
		
		
		while(count < length-1)
		{
			byte currentPixel= (byte) buff.getElem(count++);
			
			// While in a run of the same color, and run_length of char size,
			// Increment the run length and increment the data pointer to the next color
			while(((byte) buff.getElem(count) == currentPixel) && (runLength < 255) && (count < length-1))
			{
				runLength++;
				count++;
			}
		
			if(count==length-1)
			{
				runLength++;
			}
			tempBuffer.setElem(encodedLength++, runLength);
			tempBuffer.setElem(encodedLength++, currentPixel);
			runLength = 0;
		}
		
		DataBufferInt compBuffer = new DataBufferInt(encodedLength);
		
		for(int i=0; i<encodedLength; i++)
		{
			compBuffer.setElem(i,tempBuffer.getElem(i));
		}
		return(compBuffer);
	}
	
	
	
	public DataBufferInt rle_8_8bpp_mod() {
		DataBuffer buff = destination.getRaster().getDataBuffer();
		DataBufferInt tempBuffer= new DataBufferInt(buff.getSize()*3);
		
		int count=0;
		int length = buff.getSize();
		int runLength = 0;		//Store size of run length
		int encodedLength = 1;	//Store the size of compressed buffer
		int bits=0, position=1, encode=0;
		int missing=0;
		byte currentPixel=0;
		

		while(count < length-1)
		{
			
			if((runLength==0) && (missing==0)){
				currentPixel= (byte) buff.getElem(count++);
				
				// While in a run of the same color, and run_length of char size,
				// Increment the run length and increment the data pointer to the next color
				while(((byte) buff.getElem(count) == currentPixel) && (runLength < 255) && (count < length-1))
				{
					runLength++;
					count++;
				}
				if(count==length-1)
				{
					runLength++;
				}

			}
			
			//	Check if the 8 RLE bytes have been encoded and if the current
			// 	byte corresponds to run

			if ((bits==7) && (runLength>2)&& (count < length))
			{
				
				encode |= (1 << (7 - bits));
				tempBuffer.setElem(encodedLength-position, encode);
				tempBuffer.setElem(encodedLength++,runLength);
				tempBuffer.setElem(encodedLength++,currentPixel);
				runLength = 0;
				bits = 0;
				encode = 0;
				position = 1;
				missing=0;
			}
			
			//	Check if the 8 RLE bytes have been encoded and if the current
			// 	byte does not corresponds to a run
			
			else if ((bits==7) && (runLength<=2) && (count < length))
			{
				tempBuffer.setElem(encodedLength-position,encode);
				tempBuffer.setElem(encodedLength++,currentPixel);
				encodedLength++;
				runLength--;
				bits=0;
				encode = 0;
				position = 1;
				
			
				while (runLength != -1)
				{
					tempBuffer.setElem(encodedLength++,currentPixel);
					bits++;
					position ++;
					runLength--;
				}
				runLength=0;
				missing=0;
				
				
			}
			
			
			//	Check if the current byte corresponds to a run greater than 2
			else if ((runLength > 2) && (count < length))
			{
				encode |= (1 << (7 - bits));
				tempBuffer.setElem(encodedLength++,runLength);
				tempBuffer.setElem(encodedLength++,currentPixel);
				runLength = 0;
				bits++;
				position += 2;
			}
			
			
			//	Check if the current byte does not corresponds to a run
			else if ((runLength<=2) && (count < length))
			{

				while(bits<7 && runLength != -1)
				{
					tempBuffer.setElem(encodedLength++,currentPixel);
					bits++;
					position ++;
					runLength--;
				}
				
				if(runLength != -1)
				{
					missing =1;
				}
				else
				{
					runLength =0;
				}
			}
		}
					
			
			DataBufferInt temp2Buffer= new DataBufferInt(tempBuffer.getSize());
			
			// Reset the state of the encoded sequence.
			length= encodedLength;
			encodedLength = 1;
			runLength = 0;
			count = 0;
			encode=0;
			bits=0;
			position=0;
			
			boolean state = true;
			boolean get = false;
			boolean overflow =false;
			int change=0;

			
			encode= tempBuffer.getElem(count);
			
			if(BigInteger.valueOf(encode).testBit(7)== true){
				state=true;
			}else{
				state=false;
			}
			
			
			while(count < length-1)
			{
				if(bits==0 && change==0 && get==false){
					//For debug purpose
					//System.out.println("Getting next info byte: "+ Integer.toHexString(encode)+"\t" + Integer.toBinaryString(encode));

					encode= (byte) tempBuffer.getElem(count++);
				}
				else if(bits==0 && change>0)
				{
					encode= (byte) tempBuffer.getElem(count+change);
					position=change;
				}

				else if (get==true)
				{
					get=false;
				}

				else if (overflow==true)
				{		

					if(BigInteger.valueOf(encode).testBit(7- bits)== true)
						state=true;
					else
						state=false;

					overflow=false;

				}
				//Compressed Run
				if(state==true)
				{

					//For debug purpose
					//System.out.println("Entering compressed run");
					//System.out.println("Encode= " +BigInteger.valueOf(encode).testBit(7-bits)+ "\t Bits= "+ bits + "\tState = "+ state + "\tCount = "+ count);
					while((bits < 8) && (BigInteger.valueOf(encode).testBit(7-bits)==true) && (runLength < 127) && (count < length-1))
					{
						if(change > 0)
						{
							while(position!=0)
							{
								
								temp2Buffer.setElem(encodedLength++, 0);
								temp2Buffer.setElem(encodedLength++, tempBuffer.getElem(count++));
								position--;
								change--;
								runLength++;
							}
							count++;
							while(change!=0)
							{
								temp2Buffer.setElem(encodedLength++, 0);
								temp2Buffer.setElem(encodedLength++, tempBuffer.getElem(count++));
								change--;
								runLength++;
							}
						}
						temp2Buffer.setElem(encodedLength++, tempBuffer.getElem(count++));
						temp2Buffer.setElem(encodedLength++, tempBuffer.getElem(count++));
						bits++;
						runLength++;
					}
					
					//For debug purpose
					//System.out.println("Exit the compressed run");
					//System.out.println("Run length= " + runLength + "Bits= " + bits );
					
					while( (bits < 8)&&(BigInteger.valueOf(encode).testBit(7- bits)== false) && (change <= 2) && (count < length-1))		
					{
						//For debug purpose
						//System.out.println("Cheking for uncompress run greater than 3");
						
						//Uncompressed Byte Occurred, Track how many with uiChange
						change++;
						bits++;
					}
				}
				//Uncompressed Run
				else
				{
					//For debug purpose
					//System.out.println("Entering uncompressed run");
					//System.out.println("Encode= " +BigInteger.valueOf(encode).testBit(7-bits)+ "\t Bits= "+ bits + "\tState = "+ state + "\tCount = "+ count);
					while((bits<8) && (BigInteger.valueOf(encode).testBit((7-bits))== state) && (runLength<127)&& (count <= length-1))
					{
						//For debug purpose
						//System.out.println("Run Length= "+runLength + "\tBits= " + bits);
						temp2Buffer.setElem(encodedLength++, tempBuffer.getElem(count++));
						bits++;
						runLength++;
					}

					//For debug purpose
					//System.out.println("Exiting uncompressed lopp");
					//System.out.println("Run Length" +runLength+ "\tBits= "+ bits + "\tState = "+ state + "\tCount = "+ count);

					//Compressed Byte Follows Info Byte
					if((bits == 0) && (runLength < 127) && (count < length-1))
					{
						temp2Buffer.setElem(encodedLength-runLength-1, runLength);
						runLength=0;
						encodedLength++;
						state=true;
						get=true;
					}
					//Compressed Byte Occurred
					else if((bits != 8) && (runLength < 127) && (count < length-1))
					{
						
						//For debug purpose
						//System.out.println("Found a Compressed bit");
						//System.out.println("Run length= " + runLength + "Bits= " + bits + "\tCount= " +count);
						
						temp2Buffer.setElem(encodedLength-runLength-1, runLength);
						runLength=0;
						encodedLength++;
						state=true;
					}
					
					// For debug purposes
					//System.out.println("Exiting uncompressed run");
				}

				//If the number of consecutive RLE encoded bytes is greater than 127
				if((runLength == 127) && (count < length-1))
				{
					if(state == true)
					{
						temp2Buffer.setElem(encodedLength - (2 * runLength) - 1, 0x80|runLength);
						encodedLength++;
						runLength = 0;
					}
					else if (state == false)
					{
						temp2Buffer.setElem(encodedLength - runLength - 1, runLength);
						encodedLength++;
						runLength = 0;
					}

					overflow =true;
				}

				// Enter this loop if uncompressed byte needs to be transformed to RLE encoded
				else if((change <= 2) && (bits < 8) && (count < length-1) && (runLength>0) && (runLength<127))
				{
					runLength += change;
					// Did uncompressed byte begin on last info byte?
					if(position!=0)
					{
						while(position!=0)
						{
							temp2Buffer.setElem(encodedLength++, 0);
							temp2Buffer.setElem(encodedLength++, tempBuffer.getElem(count++));
							position--;
							change--;
						}
						// Skip infobyte
						count++;

						// Keep transforming uncompressed bytes
						while(change!=0)
						{
							temp2Buffer.setElem(encodedLength++, 0);
							temp2Buffer.setElem(encodedLength++, tempBuffer.getElem(count++));
							change--;
						}
					}
					else
					{
						// Uncompressed bytes are contained in current infobyte
						for(int i=0; i < change; i++)
						{
							temp2Buffer.setElem(encodedLength++, 0);
							temp2Buffer.setElem(encodedLength++, tempBuffer.getElem(count++));
						}
					}
					change = 0;
				}


				// If run is larger than 127
				else if((change <= 2) && (bits < 8) && (change +runLength >127) && (count < length-1))
				{
					// If uncompressed run is contained in previous infobyte
					if(position!=0)
					{
						while(position!=0 || runLength!=127)
						{
							temp2Buffer.setElem(encodedLength++, 0);
							temp2Buffer.setElem(encodedLength++, tempBuffer.getElem(count++));
							position--;
							change--;
							runLength++;
						}


						if(runLength == 127)
						{
							// Encode the compress run of 127
							temp2Buffer.setElem(encodedLength - (2 * runLength) - 1,0x80|runLength);
							encodedLength++;
							runLength=0;


							// Continue encoding uncompress bytes until info byte is reach
							while(position!=0)
							{
								temp2Buffer.setElem(encodedLength++, 0);
								temp2Buffer.setElem(encodedLength++, tempBuffer.getElem(count++));	
								position--;
								change--;
								runLength++;
							}

							// Skip infobyte
							count++;

							// If uncompress bytes where found after after info byte
							while(change!=0)
							{
								temp2Buffer.setElem(encodedLength++, 0);
								temp2Buffer.setElem(encodedLength++, tempBuffer.getElem(count++));
								change--;
								runLength++;
							}

						}else if(position == 0)
						{
							// Skip infobyte
							count++;

							// Continue encoding until run length is 127
							while (runLength !=127)
							{
								temp2Buffer.setElem(encodedLength++, 0);
								temp2Buffer.setElem(encodedLength++, tempBuffer.getElem(count++));
								change--;
								runLength++;
							}

							// Encode the compress run of 127
							temp2Buffer.setElem(encodedLength- (2 * runLength) - 1, 0x80|runLength );
							encodedLength++;
							runLength=0;

							// Continue encoding uncompress bytes
							while(change!=0)
							{
								temp2Buffer.setElem(encodedLength++, 0);
								temp2Buffer.setElem(encodedLength++, tempBuffer.getElem(count++));
								change--;
								runLength++;
							}
						}
					}
					// Uncompress byte to be encoded are contained on current info byte
					else
					{
						while (runLength !=127)
						{
							temp2Buffer.setElem(encodedLength++, 0);
							temp2Buffer.setElem(encodedLength++, tempBuffer.getElem(count++));
							change--;
							runLength++;
						}

						// Encode the compress run of 127
						temp2Buffer.setElem(encodedLength - (2 * runLength) - 1, 0x80|runLength);
						encodedLength++;
						runLength=0;

						// Continue encoding uncompress bytes
						while(change!=0)
						{
							temp2Buffer.setElem(encodedLength++, 0);
							temp2Buffer.setElem(encodedLength++, tempBuffer.getElem(count++));
							change--;
							runLength++;
						}

					}
				}

				// If three uncompressed bytes are found in a row
				else if((change > 2) && (count < length-1))
				{
					//For debug purpose
					//System.out.println("Entered found three uncompressed bytes in a row");
					
					temp2Buffer.setElem(encodedLength - (2 * runLength) - 1,0x80|runLength);
					encodedLength++;

					//Did uncompressed run begin on last info byte?
					if(position!=0)
					{
						while(position != 0)
						{
							temp2Buffer.setElem(encodedLength++, tempBuffer.getElem(count++));
							position--;
							change--;
						}
						//Skip Current Info Byte
						count++;
					}
					//Rest of uncompressed run on current info byte
					while(change!=0)
					{
						temp2Buffer.setElem(encodedLength++, tempBuffer.getElem(count++));
						change--;
					}
					runLength=3;
					state=false;
					//For debug purspose
					//System.out.println("Exit found three uncompressed bytes in a row");
					//System.out.println("Count= " + count);
				
				}
				
					if(bits == 8){
						// For debug purposes
						//System.out.println("Reseting bits");
						bits=0;
					}
					
			}
			

			if(runLength!=0)
			{
				if(state==true)
				{
					temp2Buffer.setElem(encodedLength - (2 * runLength) - 1, 0x80|runLength);
				}
				else
				{
					temp2Buffer.setElem(encodedLength - runLength-1, runLength);
				}
			}

		
	
			
			DataBufferInt compBuffer = new DataBufferInt(encodedLength);
			//For debug purposes
			//System.out.println("Encoded Length= " + encodedLength);
			
			for(int i=0; i<encodedLength; i++)
			{
				compBuffer.setElem(i,temp2Buffer.getElem(i));
				
			}
			
			return (compBuffer);
		}

}
