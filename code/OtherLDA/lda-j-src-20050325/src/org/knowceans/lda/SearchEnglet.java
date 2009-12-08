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
 * Created on Jan 10, 2005
 */
package org.knowceans.lda;

import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.text.DecimalFormat;
import java.text.NumberFormat;
import java.util.Hashtable;
import java.util.StringTokenizer;
import java.util.Vector;

/**
 * SearchEnglet -- a small search engine that shows the topics for a given term
 * combination.
 * 
 * @author heinrich
 */
public class SearchEnglet {

    Hashtable<String, Integer> vocab;

    private LdaModel model;

    public static void main(String[] args) {
        System.out.println("LDA tiny search engine.");

        if (args.length < 3) {
            System.out.println("Usage:");
            System.out
                .println("java org.knowceans.SearchEnglet <model_noextension> <settings_file> <vocabulary_file>");
            System.out.println("Example:");
            System.out
                .println("java org.knowceans.SearchEnglet berry.model/034 settings.txt berry/vocab.txt");
            return;
        }

        SearchEnglet se = new SearchEnglet(args[0], args[1], args[2]);
        se.search();
    }

    /**
     * initialise the SearchEnglet
     * 
     * @param modelRoot
     *            the string that all model files start with
     * @param settings
     *            the settings file (full or relative path)
     * @param the
     *            vocabulary file (full or relative path)
     */
    public SearchEnglet(String modelRoot, String settings, String vocabulary) {

        model = new LdaModel(modelRoot);
        LdaEstimate.readSettings(settings);
        loadVocabulary(vocabulary);
    }

    public void loadVocabulary(String vocabulary) {
        try {
            BufferedReader br = new BufferedReader(new FileReader(vocabulary));
            vocab = new Hashtable<String, Integer>();
            String line;
            int i = 0;
            while ((line = br.readLine()) != null) {
                if (line.length() > 0)
                    vocab.put(line.toLowerCase(), i++);
            }
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public void search() {
        try {
            BufferedReader br = new BufferedReader(new InputStreamReader(
                System.in));
            String line;
            StringTokenizer st;
            System.out.println("Query (.q to quit):");
            while ((line = br.readLine()) != null) {
                if (line.equals(".q"))
                    break;
                st = new StringTokenizer(line.toLowerCase());
                Document doc = new Document(st.countTokens());
                int ix = 0;
                while (st.hasMoreElements()) {
                    Integer a = vocab.get(st.nextElement());
                    if (a != null) {
                        doc.setWord(ix, a);
                        doc.setCount(ix, 1);
                        doc.compile();
                        ix++;
                    }
                }
                NumberFormat nf = new DecimalFormat("0.0000");
                if (doc.getLength() > 0) {

                    double[] gamma = new double[model.getNumTopics()];
                    double likelihood = infer(doc, gamma);
                    for (int i = 0; i < gamma.length; i++) {
                        System.out.println(i + " " + nf.format(gamma[i]));
                    }
                    System.out.println("(" + nf.format(likelihood) + ").");
                    System.out.println("Query (.q to exit):");
                }
            }
        } catch (IOException e) {
            e.printStackTrace();
        }

    }

    /**
     * inference only
     */
    public double infer(Document doc, double[] varGamma) {
        double[][] phi;
        double likelihood;

        phi = new double[doc.getLength()][model.getNumTopics()];
        likelihood = LdaInference.ldaInference(doc, model, varGamma, phi);
        return likelihood;
    }

}
