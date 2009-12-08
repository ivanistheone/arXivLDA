/*
 * (C) Copyright 2005, Gregor Heinrich (gregor :: arbylon : net) (This file is
 * part of the lda-j (org.knowceans.lda.*) experimental software package.)
 */
/*
 * lda-j is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 */
/*
 * lda-j is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 */
/*
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*
 * Created on Dec 3, 2004
 */
package org.knowceans.lda;

/**
 * wrapper for a document in LDA
 * <p>
 * lda-c reference: struct document in lda.h. TODO automatic length tracking
 * 
 * @author heinrich
 */
public class Document {

    private int[] words;

    private int[] counts;

    private int length;

    private int total;

    /**
     * 
     */
    public Document() {
        length = 0;
        words = new int[0];
        counts = new int[0];
    }

    /**
     * 
     */
    public Document(int length) {
        words = new int[length];
        counts = new int[length];
        this.length = length;
    }

    public void compile() {
        try {
            if (counts.length != words.length)
                throw new Exception("Document inconsistent.");
            length = counts.length;
            for (int c : counts)
                total += c;
        } catch (Exception e) {
            e.printStackTrace();
        }

    }

    /**
     * @return
     */
    public int[] getCounts() {
        return counts;
    }

    /**
     * @param index
     * @return
     */
    public int getCount(int index) {
        return counts[index];
    }

    /**
     * @param count
     * @param index
     */
    public void setCount(int index, int count) {
        counts[index] = count;
    }

    /**
     * @return
     */
    public int getLength() {
        return length;
    }

    /**
     * @return
     */
    public int getTotal() {
        return total;
    }

    /**
     * @return
     */
    public int[] getWords() {
        return words;
    }

    /**
     * @param index
     * @return
     */
    public int getWord(int index) {
        return words[index];
    }

    /**
     * @param word
     * @param index
     */
    public void setWord(int index, int word) {
        words[index] = word;
    }

    /**
     * @param is
     */
    public void setCounts(int[] is) {
        counts = is;
    }

    /**
     * @param i
     */
    public void setLength(int i) {
        length = i;
    }

    /**
     * @param i
     */
    public void setTotal(int i) {
        total = i;
    }

    /**
     * @param is
     */
    public void setWords(int[] is) {
        words = is;
    }

    /*
     * (non-Javadoc)
     * 
     * @see java.lang.Object#toString()
     */
    public String toString() {
        StringBuffer b = new StringBuffer();
        b.append("Document {length=" + length + " total=" + total + "}");
        return b.toString();
    }

}
