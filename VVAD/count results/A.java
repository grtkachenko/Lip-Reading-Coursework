import java.io.*;
import java.util.*;
import java.math.*;

public class A {
    FastScanner in;
    PrintWriter out;

    public void solve() throws IOException {
        String s;
        double subtitles = 0.0;
        double speech = 0.0;
        double all = 0.0;
        double match = 0.0;
        int q = 0;

        while (true) {
            s = in.br.readLine();

            if (s == null) {
                break;
            }

            if (s.length() < 4) {
                continue;
            }

            if (s.startsWith("Subt")) {
                subtitles += Double.parseDouble(s.substring(11, 16));
            } else if (s.startsWith("Spee")) {
                speech += Double.parseDouble(s.substring(8, 13));
            } else if (s.startsWith("Union")) {
                all += Double.parseDouble(s.substring(7, 12));
            } else if (s.startsWith("Match")) {
                match += Double.parseDouble(s.substring(7, 12));
                q++;
            }
        }

        match /= q;
        out.println(subtitles);
        out.println(speech);
        out.println(all);
        out.println(match);
    }

    public void run() {
        try {
            in = new FastScanner(new File("output.txt"));
            out = new PrintWriter(new File("new_output.out"));

            solve();

            out.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    class FastScanner {
        BufferedReader br;
        StringTokenizer st;

        FastScanner(File f) {
            try {
                br = new BufferedReader(new FileReader(f));
            } catch (FileNotFoundException e) {
                e.printStackTrace();
            }
        }

        String next() {
            while (st == null || !st.hasMoreTokens()) {
                try {
                    st = new StringTokenizer(br.readLine());
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }

            return st.nextToken();
        }

        int nextInt() {
            return Integer.parseInt(next());
        }

        long nextLong() {
            return Long.parseLong(next());
        }

        double nextDouble() {
            return Double.parseDouble(next());
        }
    }

    public static void main(String[] arg) {
        new A().run();
    }
}
