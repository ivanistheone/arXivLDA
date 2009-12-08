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

import static org.knowceans.lda.Utils.*;
import static java.lang.Math.*;
import static java.lang.Double.*;

/**
 * lda inference functions
 * <p>
 * lda-c reference: functions in lda-inference.c. TODO: merge with model?
 * 
 * @author heinrich
 */
public class LdaInference {

    static float VAR_CONVERGED;

    static int VAR_MAX_ITER;

    /*
     * variational inference
     */
    public static double ldaInference(Document doc, LdaModel model,
        double[] varGamma, double[][] phi) {
        double converged = 1;
        double phisum = 0, likelihood = 0, likelihoodOld = Double.NEGATIVE_INFINITY;
        double[] oldphi = new double[model.getNumTopics()];
        int k, n, varIter;

        assert model.getNumTopics() > 0;
        for (k = 0; k < model.getNumTopics(); k++) {
            varGamma[k] = model.getAlpha() + doc.getTotal()
                / (double) model.getNumTopics();
            for (n = 0; n < doc.getLength(); n++)
                phi[n][k] = 1.0 / model.getNumTopics();
        }
        varIter = 0;
        while ((converged > VAR_CONVERGED) && (varIter < VAR_MAX_ITER)) {
            varIter++;
            for (n = 0; n < doc.getLength(); n++) {
                phisum = 0;
                for (k = 0; k < model.getNumTopics(); k++) {
                    oldphi[k] = phi[n][k];
                    assert varGamma[k] != 0;
                    if (model.getClassWord(k, doc.getWord(n)) > 0) {
                        assert model.getClassTotal(k) != 0;
                        phi[n][k] = digamma(varGamma[k])
                            + log(model.getClassWord(k, doc.getWord(n)))
                            - log(model.getClassTotal(k));
                    } else {
                        phi[n][k] = digamma(varGamma[k]) - 100;
                    }
                    if (k > 0) {
                        phisum = logSum(phisum, phi[n][k]);
                    } else {
                        phisum = phi[n][k];
                    }
                }
                for (k = 0; k < model.getNumTopics(); k++) {
                    phi[n][k] = exp(phi[n][k] - phisum);
                    varGamma[k] = varGamma[k] + doc.getCount(n)
                        * (phi[n][k] - oldphi[k]);
                }
            }
            likelihood = computeLikelihood(doc, model, phi, varGamma);
            assert likelihoodOld != 0;
            converged = (likelihoodOld - likelihood) / likelihoodOld;
            likelihoodOld = likelihood;
        }
        return likelihood;
    }

    /*
     * compute likelihood bound
     */
    public static double computeLikelihood(Document doc, LdaModel model,
        double[][] phi, double[] varGamma) {
        double likelihood = 0, digsum = 0, varGammaSum = 0;// , x;
        double[] dig = new double[model.getNumTopics()];
        int k, n, message = 0;

        for (k = 0; k < model.getNumTopics(); k++) {
            dig[k] = digamma(varGamma[k]);
            varGammaSum += varGamma[k];
        }
        digsum = digamma(varGammaSum);
        likelihood = lgamma(model.getAlpha() * model.getNumTopics())
            - model.getNumTopics() * lgamma(model.getAlpha())
            - (lgamma(varGammaSum));
        assert likelihood != Double.NaN;
        if ((isNaN(likelihood)) && (message == 0)) {
            // printf("(1) : %5.5f %5.5f\n", var_gamma_sum, likelihood);
            System.out.println("(1) : " + varGammaSum + " " + likelihood);
            message = 1;
        }
        for (k = 0; k < model.getNumTopics(); k++) {
            likelihood += (model.getAlpha() - 1) * (dig[k] - digsum)
                + lgamma(varGamma[k]) - (varGamma[k] - 1) * (dig[k] - digsum);
            assert likelihood != Double.NaN;
            if ((isNaN(likelihood)) && (message == 0)) {
                // printf("(2 %d) : %5.5f\n", k, likelihood);
                System.out.println("(2 " + k + ") : " + likelihood);
                message = 1;
            }
            for (n = 0; n < doc.getLength(); n++) {
                if (model.getClassWord(k, doc.getWord(n)) > 0) {
                    if (phi[n][k] > 0) {
                        // likelihood += doc->counts[n]*
                        // (phi[n][k]*((dig[k] - digsum) - log(phi[n][k])
                        // + log(model->class_word[k][doc->words[n]])
                        // - log(model->class_total[k])));
                        assert phi[n][k] > 0;
                        assert model.getClassTotal(k) > 0;
                        likelihood += doc.getCount(n)
                            * (phi[n][k] * ((dig[k] - digsum) - log(phi[n][k])
                                + log(model.getClassWord(k, doc.getWord(n))) - log(model
                                .getClassTotal(k))));
                        assert likelihood != Double.NaN;
                        if ((isNaN(likelihood)) && (message == 0)) {
                            // printf("(2 %d) : %5.5f\n", k, likelihood);
                            // printf("(3 %d %d) : %5.5f\n", k, n, likelihood);
                            System.out.println("(2 " + k + ") : " + likelihood);
                            System.out.println("(3 " + k + " " + n + ") : "
                                + likelihood);
                        }
                    }
                }
            }
        }
        return likelihood;
    }
}
