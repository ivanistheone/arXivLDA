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
 * Created on Jan 4, 2005
 */
package org.knowceans.lda;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import static java.lang.Math.*;
import java.text.DecimalFormat;
import java.text.NumberFormat;

import org.knowceans.util.CokusNative;

/**
 * lda parameter estimation
 * <p>
 * lda-c reference: functions in lda-estimate.c.
 * 
 * @author heinrich
 */
public class LdaEstimate {

    /*
     * For remote debugging: -Xdebug -Xnoagent
     * -Xrunjdwp:transport=dt_socket,server=y,suspend=n,address=
     * <MyJdwpDebugPort>
     */
    static int LAG = 10;

    static int NUM_INIT = 1;

    static float EM_CONVERGED;

    static int EM_MAX_ITER;

    static int ESTIMATE_ALPHA;

    static double INITIAL_ALPHA;

    static double K;

    static {
        CokusNative.cokusseed(4357);
    }

    static double myrand() {
        return (((long) CokusNative.cokusrand()) & 0xffffffffl)
            / (double) 0x100000000l;

    }

    /**
     * initializes class_word and class_total to reasonable beginnings.
     */
    public static LdaModel initialModel(String start, Corpus corpus,
        int numTopics, double alpha) {
        int k, d, i, n;
        LdaModel model;
        Document doc;

        if (start.equals("seeded")) {
            model = new LdaModel(corpus.getNumTerms(), numTopics);
            model.setAlpha(alpha);
            // foreach topic
            for (k = 0; k < numTopics; k++) {
                // sample NUM_INIT documents and add their term counts to the
                // class-word table
                for (i = 0; i < NUM_INIT; i++) {
                    d = (int) floor(myrand() * corpus.getNumDocs());
                    System.out.println("initialized with document " + d);
                    doc = corpus.getDoc(d);
                    for (n = 0; n < doc.getLength(); n++) {
                        model.addClassWord(k, doc.getWord(n), doc.getCount(n));
                    }
                }
                // add to all terms in class-word table 1/nTerms; update class
                // total accordingly
                assert model.getNumTerms() > 0;
                for (n = 0; n < model.getNumTerms(); n++) {
                    model.addClassWord(k, n, 1.0 / model.getNumTerms());
                    model.addClassTotal(k, model.getClassWord(k, n));
                }
            }
        } else if (start.equals("random")) {
            model = new LdaModel(corpus.getNumTerms(), numTopics);
            model.setAlpha(alpha);
            // for each topic
            for (k = 0; k < numTopics; k++) {
                // add to all terms in class-word table a random sample \in
                // (0,1) plus 1/nTerms;
                // update class total accordingly
                for (n = 0; n < model.getNumTerms(); n++) {
                    model.addClassWord(k, n, 1.0 / model.getNumTerms()
                        + myrand());
                    model.addClassTotal(k, model.getClassWord(k, n));
                }
            }
        } else {
            // load model from file (.beta and .other data)
            model = new LdaModel(start);
        }
        return model;
    }

    /**
     * iterate_document
     */
    public static double docEm(Document doc, double[] gamma, LdaModel model,
        LdaModel nextModel) {
        double likelihood;
        double[][] phi;
        int n, k;

        phi = new double[doc.getLength()][model.getNumTopics()];

        likelihood = LdaInference.ldaInference(doc, model, gamma, phi);
        for (n = 0; n < doc.getLength(); n++) {
            for (k = 0; k < model.getNumTopics(); k++) {
                nextModel.addClassWord(k, doc.getWord(n), doc.getCount(n)
                    * phi[n][k]);
                nextModel.addClassTotal(k, doc.getCount(n) * phi[n][k]);
            }
        }
        return likelihood;
    }

    /**
     * saves the gamma parameters of the current dataset
     */
    static void saveGamma(String filename, double[][] gamma, int numDocs,
        int numTopics) {
        try {
            BufferedWriter bw = new BufferedWriter(new FileWriter(filename));
            int d, k;
            for (d = 0; d < numDocs; d++) {
                for (k = 0; k < numTopics; k++) {
                    if (k > 0)
                        bw.write(' ');
                    bw.write(Utils.formatDouble(gamma[d][k]));
                }
                bw.newLine();
            }
            bw.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    /**
     * run_em
     */
    public static LdaModel runEm(String start, String directory, Corpus corpus) {
        try {
            BufferedWriter likelihoodFile;
            String filename;
            int i, d;
            double likelihood, likelihoodOld = Double.NEGATIVE_INFINITY, converged = 1;
            LdaModel model, nextModel;
            double[][] varGamma;
            filename = directory + "/" + "likelihood.dat";
            likelihoodFile = new BufferedWriter(new FileWriter(filename));
            varGamma = new double[corpus.getNumDocs()][(int) K];
            model = initialModel(start, corpus, (int) K, INITIAL_ALPHA);
            filename = directory + "/000";
            model.save(filename);
            i = 0;
            NumberFormat nf = new DecimalFormat("000");
            String itername = "";
            while (((converged > EM_CONVERGED) || (i <= 2))
                && (i <= EM_MAX_ITER)) {
                i++;
                System.out.println("**** em iteration " + i + " ****");
                likelihood = 0;
                nextModel = new LdaModel(model.getNumTerms(), model
                    .getNumTopics());
                nextModel.setAlpha(INITIAL_ALPHA);
                for (d = 0; d < corpus.getNumDocs(); d++) {
                    if ((d % 100) == 0)
                        System.out.println("document " + d);
                    likelihood += docEm(corpus.getDoc(d), varGamma[d], model,
                        nextModel);
                }
                if (ESTIMATE_ALPHA == 1)
                    LdaAlpha.maximizeAlpha(varGamma, nextModel, corpus
                        .getNumDocs());
                model.free();
                model = nextModel;
                assert likelihoodOld != 0;
                converged = (likelihoodOld - likelihood) / likelihoodOld;
                likelihoodOld = likelihood;
                // fprintf(likelihood_file, "%10.10f\t%5.5e\n", likelihood,
                // converged);
                likelihoodFile.write(likelihood + "\t" + converged + "\n");
                likelihoodFile.flush();

                if ((i % LAG) == 0) {
                    // sprintf(filename,"%s/%03d",directory, i);
                    itername = nf.format(i);
                    filename = directory + "/" + itername;
                    model.save(filename);

                    filename = directory + "/" + itername + ".gamma";
                    saveGamma(filename, varGamma, corpus.getNumDocs(), model
                        .getNumTopics());
                }
            }
            itername = nf.format(i);
            filename = directory + "/" + itername;
            model.save(filename);
            filename = directory + "/" + itername + ".gamma";
            saveGamma(filename, varGamma, corpus.getNumDocs(), model
                .getNumTopics());
            likelihoodFile.close();
            return model;
        } catch (IOException e) {
            e.printStackTrace();
            return null;
        }
    }

    /**
     * read settings.
     */
    public static void readSettings(String filename) {
        String alphaAction = "";

        BufferedReader br;
        try {
            br = new BufferedReader(new FileReader(filename));
            String line;
            while ((line = br.readLine()) != null) {

                if (line.startsWith("var max iter ")) {
                    LdaInference.VAR_MAX_ITER = Integer.parseInt(line
                        .substring(13).trim());
                } else if (line.startsWith("var convergence ")) {
                    LdaInference.VAR_CONVERGED = Float.parseFloat(line
                        .substring(16).trim());
                } else if (line.startsWith("em max iter ")) {
                    EM_MAX_ITER = Integer.parseInt(line.substring(12).trim());
                } else if (line.startsWith("em convergence ")) {
                    EM_CONVERGED = Float.parseFloat(line.substring(15).trim());
                } else if (line.startsWith("alpha ")) {
                    alphaAction = line.substring(6).trim();
                }
            }
            br.close();
        } catch (NumberFormatException e) {
            e.printStackTrace();
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }

        if (alphaAction.equals("fixed")) {
            ESTIMATE_ALPHA = 0;
        } else {
            ESTIMATE_ALPHA = 1;
        }
    }

    /**
     * inference only
     */
    public static void infer(String modelRoot, String save, Corpus corpus) {
        String filename;
        int i, d, n;
        LdaModel model;
        double[][] varGamma, phi;
        double likelihood;
        Document doc;

        model = new LdaModel(modelRoot);
        varGamma = new double[corpus.getNumDocs()][model.getNumTopics()];
        filename = save + ".likelihoods";

        try {
            BufferedWriter bw = new BufferedWriter(new FileWriter(filename));
            for (d = 0; d < corpus.getNumDocs(); d++) {
                if ((d % 100) == 0)
                    System.out.println("document " + d);

                doc = corpus.getDoc(d);
                phi = new double[doc.getLength()][model.getNumTopics()];
                likelihood = LdaInference.ldaInference(doc, model, varGamma[d],
                    phi);

                bw.write(Utils.formatDouble(likelihood));
                bw.newLine();
            }
            bw.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
        filename = save + ".gamma";
        saveGamma(filename, varGamma, corpus.getNumDocs(), model.getNumTopics());
    }

    /**
     * main
     */
    public static void main(String[] args) {

        int i = 0;
        double y = 0;
        double x, z, d;
        Corpus corpus;

        // command: lda est 0.10 16 settings.txt berry/berry.dat seeded
        // berry.model
        if (args[0].equals("est")) {
            if (args.length < 7) {
                System.out
                    .println("usage\n: lda est <initial alpha> <k> <settings> <data> <random/seeded/*> <directory>");
                System.out
                    .println("example\n: lda est 10 100 settings.txt ..\\ap\\ap.dat seeded ..\\ap.model");
                return;
            }

            INITIAL_ALPHA = Float.parseFloat(args[1]);
            K = Integer.parseInt(args[2]);
            readSettings(args[3]);
            corpus = new Corpus(args[4]);
            boolean a = new File(args[6]).mkdir();

            System.out.println("LDA estimation. Settings:");
            System.out.println("\tvar max iter " + LdaInference.VAR_MAX_ITER);
            System.out.println("\tvar convergence "
                + LdaInference.VAR_CONVERGED);
            System.out.println("\tem max iter " + EM_MAX_ITER);
            System.out.println("\tem convergence " + EM_CONVERGED);
            System.out.println("\testimate alpha " + ESTIMATE_ALPHA);

            LdaModel model = runEm(args[5], args[6], corpus);
            // model.getClassWord();

        } else {
            // command: lda inf settings.txt berry.model berry/berry.dat
            // berry.inf
            if (args.length < 5) {
                System.out
                    .println("usage\n: lda inf <settings> <model> <data> <name>\n");
                System.out
                    .println("example\n: lda inf settings.txt ..\\ap.model ..\\aptest ..\\aptest.inf\n");
                return;
            }
            readSettings(args[1]);

            System.out.println("LDA inference. Settings:");
            System.out.println("\tvar max iter " + LdaInference.VAR_MAX_ITER);
            System.out.println("\tvar convergence "
                + LdaInference.VAR_CONVERGED);
            System.out.println("\tem max iter " + EM_MAX_ITER);
            System.out.println("\tem convergence " + EM_CONVERGED);
            System.out.println("\testimate alpha " + ESTIMATE_ALPHA);

            corpus = new Corpus(args[3]);

            infer(args[2], args[4], corpus);

        }
    }
}
