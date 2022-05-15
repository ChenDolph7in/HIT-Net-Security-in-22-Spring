package Socket;

import Utils.Utils;

import javax.swing.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.net.Socket;
import java.util.ArrayList;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class Sniffer extends Thread {
    private Socket socket;

    @Override
    public void run() {
        JFrame jf = new JFrame("Sniffer");
        jf.setSize(600, 400);
        jf.setDefaultCloseOperation(WindowConstants.EXIT_ON_CLOSE);
        jf.setLocationRelativeTo(null);

        // 创建面板
        JPanel panel = new JPanel();
        panel.setLayout(null);    // 面板布局

        // 创建 标签 & 输入框 & 按钮
        JLabel IPField = new JLabel("ip范围:");
        JLabel PortField = new JLabel("端口范围:");
        JLabel ThreadField = new JLabel("线程数:");
        JLabel Point1 = new JLabel(".");
        JLabel Point2 = new JLabel(".");
        JLabel Point3 = new JLabel(".");
        JLabel Point4 = new JLabel(".");
        JLabel Point5 = new JLabel(".");
        JLabel Point6 = new JLabel(".");
        JLabel Bar1 = new JLabel("——");
        JLabel Bar2 = new JLabel("——");
        JTextField StartIP1 = new JTextField(10);
        JTextField StartIP2 = new JTextField(10);
        JTextField StartIP3 = new JTextField(10);
        JTextField StartIP4 = new JTextField(10);
        JTextField EndIP1 = new JTextField(10);
        JTextField EndIP2 = new JTextField(10);
        JTextField EndIP3 = new JTextField(10);
        JTextField EndIP4 = new JTextField(10);
        JTextField StartPort = new JTextField(10);
        JTextField EndPort = new JTextField(10);
        JTextField ThreadNum = new JTextField(10);
        JButton loginButton = new JButton("开始扫描");


        // 设置标签的大小和位置
        IPField.setBounds(50, 50, 50, 20);
        StartIP1.setBounds(100, 50, 40, 20);
        Point1.setBounds(140, 50, 20, 20);
        StartIP2.setBounds(160, 50, 40, 20);
        Point2.setBounds(200, 50, 20, 20);
        StartIP3.setBounds(220, 50, 40, 20);
        Point3.setBounds(260, 50, 20, 20);
        StartIP4.setBounds(280, 50, 40, 20);
        Bar1.setBounds(320, 50, 30, 20);
        EndIP1.setBounds(350, 50, 40, 20);
        Point4.setBounds(390, 50, 20, 20);
        EndIP2.setBounds(410, 50, 40, 20);
        Point5.setBounds(450, 50, 20, 20);
        EndIP3.setBounds(470, 50, 40, 20);
        Point6.setBounds(510, 50, 20, 20);
        EndIP4.setBounds(530, 50, 40, 20);
        PortField.setBounds(50, 100, 60, 20);
        StartPort.setBounds(120, 100, 100, 20);
        Bar2.setBounds(220, 100, 30, 20);
        EndPort.setBounds(250, 100, 100, 20);
        ThreadField.setBounds(50, 150, 60, 20);
        ThreadNum.setBounds(120, 150, 100, 20);
        loginButton.setBounds(230, 230, 100, 25);

        // 设置面板内容
        panel.add(IPField);
        panel.add(StartIP1);
        panel.add(StartIP2);
        panel.add(StartIP3);
        panel.add(StartIP4);
        panel.add(EndIP1);
        panel.add(EndIP2);
        panel.add(EndIP3);
        panel.add(EndIP4);
        panel.add(PortField);
        panel.add(StartPort);
        panel.add(EndPort);
        panel.add(ThreadField);
        panel.add(ThreadNum);
        panel.add(loginButton);
        panel.add(Point1);
        panel.add(Point2);
        panel.add(Point3);
        panel.add(Point4);
        panel.add(Point5);
        panel.add(Point6);
        panel.add(Bar1);
        panel.add(Bar2);
        // 将面板加入到窗口中
        jf.add(panel);

        // 监听确认按钮
        loginButton.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                boolean flag = true;
                String Startip1 = StartIP1.getText();
                String Startip2 = StartIP2.getText();
                String Startip3 = StartIP3.getText();
                String Startip4 = StartIP4.getText();
                String Endip1 = EndIP1.getText();
                String Endip2 = EndIP2.getText();
                String Endip3 = EndIP3.getText();
                String Endip4 = EndIP4.getText();
                // 检查输入是否为数字
                if (!(Utils.isNumeric(Startip1) && Utils.isNumeric(Startip2) && Utils.isNumeric(Startip3) && Utils.isNumeric(Startip4) && Utils.isNumeric(Endip1) && Utils.isNumeric(Endip2) && Utils.isNumeric(Endip3) && Utils.isNumeric(Endip4))) {
                    error("ip输入格式错误");flag = false;
                }

                String Startport = StartPort.getText();
                String Endport = EndPort.getText();
                String Threadnum = ThreadNum.getText();
                if (!(Utils.isNumeric(Startport) && Utils.isNumeric(Endport) && Utils.isNumeric(Threadnum))) {
                    error("端口或线程数输入格式错误");flag = false;
                }

                if(flag){
                    // 检查输入范围是否符合规则
                    if (Integer.valueOf(Startport) > Integer.valueOf(Endport)) {
                        error("端口范围错误");flag = false;
                    } else if (Integer.valueOf(Startip1) > Integer.valueOf(Endip1)) {
                        error("ip范围错误");flag = false;
                    } else if (Integer.valueOf(Startip1) == Integer.valueOf(Endip1)) {
                        if (Integer.valueOf(Startip2) > Integer.valueOf(Endip2)) {
                            error("ip范围错误");flag = false;
                        } else if (Integer.valueOf(Startip2) == Integer.valueOf(Endip2)) {
                            if (Integer.valueOf(Startip3) > Integer.valueOf(Endip3)) {
                                error("ip范围错误");flag = false;
                            } else if (Integer.valueOf(Startip3) == Integer.valueOf(Endip3)) {
                                if (Integer.valueOf(Startip4) > Integer.valueOf(Endip4)) {
                                    error("ip范围错误");flag = false;
                                }
                            }
                        }
                    }
                }

                if(flag){
                    long startTime = System.currentTimeMillis();
                    ArrayList<IPPort> ports = new ArrayList<>();
                    ExecutorService pool = Executors.newFixedThreadPool(Integer.valueOf(Threadnum));
                    for (int i = Integer.valueOf(Startip1); i <= Integer.valueOf(Endip1); i++) {
                        for (int j = Integer.valueOf(Startip2); j <= Integer.valueOf(Endip2); j++) {
                            for (int m = Integer.valueOf(Startip3); m <= Integer.valueOf(Endip3); m++) {
                                for (int n = Integer.valueOf(Startip4); n <= Integer.valueOf(Endip4); n++) {
                                    String IP = String.valueOf(i) + "." + String.valueOf(j) + "." + String.valueOf(m) + "." + String.valueOf(n);
                                    for (int p = Integer.valueOf(Startport); p <= Integer.valueOf(Endport); p++) {
                                        pool.execute(new SonThread(IP, p, ports));
                                    }
                                }
                            }
                        }
                    }
                    String resultString;
                    pool.shutdown();
                    while (true) {//等待所有任务都执行结束
                        if (pool.isTerminated()) {//所有的子线程都结束了
                            resultString = "共耗时:" + (System.currentTimeMillis() - startTime) / 1000.0 + "s\n";
                            System.out.println(resultString);
                            break;
                        }
                    }

                    for (IPPort ipport : ports) {
                        String follow = "[host]" + ipport.getHost() + " [port]" + ipport.getPort() + "\t:succeed\n";
                        resultString = resultString + follow;
                        System.out.println(follow);
                    }
                    result(resultString);
                }
            }
        });

        // 设置窗口可见
        jf.setVisible(true);
    }

    /**
     * @description:错误界面
     * @param:[errorString]
     * @return:void
     */
    private static void error(String errorString) {
        JFrame jf = new JFrame("Error");
        jf.setSize(400, 200);
        jf.setDefaultCloseOperation(WindowConstants.EXIT_ON_CLOSE);
        jf.setLocationRelativeTo(null);

        JPanel panel = new JPanel();
        panel.setLayout(null);
        JLabel jl = new JLabel(errorString,JLabel.CENTER);
        JButton jb = new JButton("确定");

        jl.setBounds(40, 30, 300, 30);
        jb.setBounds(155, 100, 80, 30);

        panel.add(jl);
        panel.add(jb);

        // 监听确认按钮
        jb.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                jf.dispose();
            }
        });

        jf.add(panel);
        jf.setVisible(true);
    }

    /**
     * @description:结果界面
     * @param:[resultString]
     * @return:void
     */
    private static void result(String resultString) {
        JFrame jf = new JFrame("Result");
        jf.setSize(500, 340);
        jf.setDefaultCloseOperation(WindowConstants.EXIT_ON_CLOSE);
        jf.setLocationRelativeTo(null);

        JPanel panel = new JPanel();
        panel.setLayout(null);
        //Label jl = new JLabel(resultString);
        JButton jb = new JButton("确定");
        JTextArea jt = new JTextArea(resultString, 16, 16);

        jt.setBounds(100,30,300,200);
        jb.setBounds(210, 240, 80, 30);

        JScrollPane scrollPane = new JScrollPane();
        scrollPane.setBounds(100,30,300,200);
        scrollPane.setViewportView(jt);

        panel.add(scrollPane);
        panel.add(jb);

        // 监听确认按钮
        jb.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                jf.dispose();
            }
        });

        jf.add(panel);
        jf.setVisible(true);
    }

    public static void main(String[] args) {
        Sniffer s = new Sniffer();
        s.run();
    }
}
