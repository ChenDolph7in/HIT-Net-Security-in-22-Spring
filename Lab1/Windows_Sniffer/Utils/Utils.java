package Utils;

import java.util.regex.Pattern;

public class Utils {

    /**
     * @description:判断输入是否为非负整数
     * @param:[string]
     * @return:boolean
     */
    public static boolean isNumeric(String string) {
        Pattern pattern = Pattern.compile("^[1-9]\\d*|0$");
        return pattern.matcher(string).matches();
    }
}
