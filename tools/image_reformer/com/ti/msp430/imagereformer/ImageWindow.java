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

import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.ScrollPane;
import java.awt.image.BufferedImage;

import javax.swing.JLabel;
import javax.swing.JPanel;


public class ImageWindow extends JPanel {
	private static final long serialVersionUID = -2458153725453118876L;
	BufferedImage image;
	ImageLabel jb;
	Dimension size = new Dimension();

	class ImageLabel extends JLabel {

		/**
		 * 
		 */
		private static final long serialVersionUID = 987187100574473911L;

		protected void paintComponent(Graphics g) {
			// Center image in this component.
			//super.paint(g);
			if (image != null) {
				int x = (getWidth() - size.width) / 2;
				int y = (getHeight() - size.height) / 2;
				g.drawImage(image, x, y, this);
			}
		}

		public Dimension getPreferredSize() {
			return size;
		}

	}

	public ImageWindow() {

	}

	public ImageWindow(BufferedImage im) {
		this();
		jb = new ImageLabel();
		image = im;
		if (image != null)
			size.setSize(image.getWidth(), image.getHeight());
		else
		   size.setSize(400, 400);

		this.add(jb);


	}

}
