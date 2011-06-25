package com.app.lifeNet;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;

/**
 *
 * @author hrushi
 */
public class LifeNetApi {

    static String gnstFile;
    static boolean initFlag = false;

    public static void init(String gnstFilePath) {
        gnstFile = gnstFilePath;
        initFlag = true;
    }

    public static String getNumTxNumRx(String name1) {

        StringBuffer sBuff = new StringBuffer("");
        /*
        if (initFlag) {

        try {



        BufferedReader in = new BufferedReader(new FileReader(
        gnstFile));
        String str;

        while ((str = in.readLine()) != null) {
        String[] strArr = str.split(" ");
        if (strArr[0].trim().equals(name1)) {
        in.close();

        int rxSession = Integer.parseInt(strArr[9].trim());
        int txSession = Integer.parseInt(strArr[17].trim());

        // This is commented because returning -1 causes problems in display
        //if (rxSession == txSession) {
        int numRx = Integer.parseInt(strArr[13].trim());
        int numTx = Integer.parseInt(strArr[11].trim());
        sBuff.append(numTx);
        sBuff.append(",");
        sBuff.append(numRx);
        }
        }
        in.close();

        } catch (IOException e) {
        }
        }
         *
         */
        return sBuff.toString();
    }

    public static int getED(String name1) {

        int ed = -1;

        if (initFlag) {

            try {

                BufferedReader in = new BufferedReader(new FileReader(
                        gnstFile));
                String str;

                while ((str = in.readLine()) != null) {
                    String[] strArr = str.split(" ");
                    if (strArr[0].trim().equals(name1)) {
                        in.close();

                        ed = Integer.parseInt(strArr[6].trim());
                    }
                }
                in.close();

            } catch (IOException e) {
            }
        }
        return ed;
    }

    public static String getMyName() {

        if (initFlag) {

            try {
                BufferedReader in = new BufferedReader(
                        new FileReader(gnstFile));
                String str;

                while ((str = in.readLine()) != null) {
                    String[] strArr = str.split(" ");
                    if (strArr[1].trim().equals(strArr[2].trim())) {
                        in.close();
                        return strArr[0].trim();
                    }
                }
                in.close();
            } catch (IOException e) {
            }

        }
        return "";
    }

    public static String getMyIp() {

        if (initFlag) {

            try {
                BufferedReader in = new BufferedReader(
                        new FileReader(gnstFile));
                String str;

                while ((str = in.readLine()) != null) {
                    String[] strArr = str.split(" ");
                    if (strArr[1].trim().equals(strArr[2].trim())) {
                        in.close();
                        return strArr[3].trim();
                    }
                }
                in.close();
            } catch (IOException e) {
            }

        }
        return "";
    }

    public static String getIpFromName(String name) {

        if (initFlag) {

            try {
                BufferedReader in = new BufferedReader(
                        new FileReader(gnstFile));
                String str;

                while ((str = in.readLine()) != null) {
                    String[] strArr = str.split(" ");
                    if (strArr[0].trim().equals(name)) {
                        in.close();
                        return strArr[3].trim();
                    }
                }
                in.close();
            } catch (IOException e) {
            }

        }
        return "";
    }
}
