#include <Arduino.h>

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ; // Chờ tới khi Serial hoạt động
  }
  char arraya[] = {'à', 'á'};
  String vietnameseWithDiacritics = "Anh đéo à";
  // String rs = "";
  // for (int i = 0; i < vietnameseWithDiacritics.length(); i++) {
  //   rs +=vietnameseWithDiacritics[i];
  // }
  // Serial.println(rs);
  String vietnameseWithoutDiacritics = removeDiacritics(vietnameseWithDiacritics);

  Serial.println(vietnameseWithoutDiacritics);
}

void loop() {
  // Không cần làm gì trong hàm loop
}

String removeDiacritics(String input) {
  Serial.println(input);
  // Dấu và ký tự tương ứng
  String diacritics = "à,á,ả,ã,ạ,â,ầ,ấ,ẩ,ẫ,ậ,ă,ằ,ắ,ẳ,ẵ,ặ,è,é,ẻ,ẽ,ẹ,ê,ề,ế,ể,ễ,ệ,ì,í,ỉ,ĩ,ị,ò,ó,ỏ,õ,ọ,ô,ồ,ố,ổ,ỗ,ộ,ơ,ờ,ớ,ở,ỡ,ợ,ù,ú,ủ,ũ,ụ,ư,ừ,ứ,ử,ữ,ự,ỳ,ý,ỷ,ỹ,ỵ,đ,À,Á,Ả,Ã,Ạ,Â,Ầ,Ấ,Ẩ,Ẫ,Ậ,Ă,Ằ,Ắ,Ẳ,Ẵ,Ặ,È,É,Ẻ,Ẽ,Ẹ,Ê,Ề,Ế,Ể,Ễ,Ệ,Ì,Í,Ỉ,Ĩ,Ị,Ò,Ó,Ỏ,Õ,Ọ,Ô,Ồ,Ố,Ổ,Ỗ,Ộ,Ơ,Ờ,Ớ,Ở,Ỡ,Ợ,Ù,Ú,Ủ,Ũ,Ụ,Ư,Ừ,Ứ,Ử,Ữ,Ự,Ỳ,Ý,Ỷ,Ỹ,Ỵ,Đ";
  String withoutDiacritics = "a,a,a,a,a,a,a,a,a,a,a,a,a,a,a,a,a,a,e,e,e,e,e,e,e,e,e,e,e,i,i,i,i,i,o,o,o,o,o,o,o,o,o,o,o,o,o,o,o,o,o,u,u,u,u,u,u,u,u,u,u,u,y,y,y,y,y,d,A,A,A,A,A,A,A,A,A,A,A,A,A,A,A,A,A,E,E,E,E,E,E,E,E,E,E,E,I,I,I,I,I,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,U,U,U,U,U,U,U,U,U,U,U,Y,Y,Y,Y,Y,D";
    Serial.println(diacritics.length());
    Serial.println(withoutDiacritics.length());
  // Tạo một chuỗi mới để lưu trữ kết quả
  String result = "";
  String rs = "";
  Serial.println(countSpecialCharacters(diacritics.substring(98), ','));
  // Duyệt qua từng ký tự trong chuỗi đầu vào
  for (int i = 0; i < input.length(); i++) {
    boolean isExist = false;
    rs +=input[i];
    for (int j = 0; j < diacritics.length(); j++) {
       if (input[i] == diacritics[j]) {
         char c = input[i];
         //Serial.print();
         Serial.print(j);
         char k = withoutDiacritics[j];
    //     Serial.println();
         //Serial.print(diacritics[j]);
    //     // Nếu là dấu, thay thế bằng ký tự không dấu tương ứng
         result += k;
         isExist = true;
         break;
       }
    }
    // Nếu không phải dấu, thêm vào chuỗi kết quả
    if (!isExist) {
      result += input[i];
    }

  }
  Serial.println(rs);
  // Trả về chuỗi đã được xử lý
  return result;
}
int countSpecialCharacters(const String& str, char c) {
    int count = 0; // Số lượng ký tự đặc biệt
//Serial.print(str);
    for (int i = 0; i < str.length(); ++i) {
        // Kiểm tra xem ký tự có phải là ký tự đặc biệt không
        if (str[i] == c) {
            count++;
        }
    }

    return count;
}
// Hàm phân tách chuỗi thành mảng các chuỗi con dựa trên một ký tự đặc biệt
void splitString(const String& str, char delimiter, String* tokens, int maxTokens) {
    int tokenIndex = 0; // Chỉ số của token hiện tại
    int startIndex = 0; // Chỉ số bắt đầu của token hiện tại
    
    // Duyệt qua từng ký tự trong chuỗi
    for (int i = 0; i < str.length(); ++i) {
        // Nếu gặp ký tự phân cách
        if (str[i] == delimiter) {
            // Thêm token mới vào mảng tokens
            tokens[tokenIndex++] = str.substring(startIndex, i);
            startIndex = i + 1; // Cập nhật chỉ số bắt đầu của token mới
            // Nếu đã đạt đến số lượng token tối đa được xác định, thoát vòng lặp
            if (tokenIndex >= maxTokens) break;
        }
    }
    // Thêm token cuối cùng vào mảng tokens
    tokens[tokenIndex++] = str.substring(startIndex);
}

// void tach() {

// char delimiter = ',';
//     int maxTokens = 6; // Số lượng token tối đa (số lượng phần tử trong mảng tokens)
//     String tokens[maxTokens]; // Mảng để lưu trữ các token
    
//     // Phân tách chuỗi và lưu kết quả vào mảng tokens
//     splitString(inputString, delimiter, tokens, maxTokens);
	
// 	}
