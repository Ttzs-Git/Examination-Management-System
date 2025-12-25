# analyze_exam.py
import sys
import json
import os
from openai import OpenAI
from dotenv import load_dotenv
# AI分析微服务
load_dotenv() 
api_key = os.environ.get("ARK_API_KEY")
ENDPOINT_ID = "doubao-seed-1-6-flash-250828" 

if not api_key or ENDPOINT_ID == "<ENDPOINT_ID>":
    print("\n[AI配置错误]: 请在终端设置 ARK_API_KEY 环境变量，并在脚本中填写你的 ENDPOINT_ID。")
    sys.exit(1)
try:
    client = OpenAI(
        base_url="https://ark.cn-beijing.volces.com/api/v3",
        api_key=api_key,
    )
except Exception as e:
    print(f"\n[AI客户端初始化错误]: {e}")
    sys.exit(1)


def analyze_performance(exam_data):
    """构造Prompt并调用火山方舟的豆包模型"""
    prompt_content = f"""
    你是一名资深的C语言编程辅导老师。一位学生刚刚完成了一场C语言在线测试，
    以下是他的答题记录 (JSON格式):
    
    ```json
    {json.dumps(exam_data, indent=2, ensure_ascii=False)}
    ```

    请你基于以上数据，为这位学生生成一份简洁、专业且有建设性的学习评估报告。
    报告必须包含以下几个部分，并使用文本格式化(使用txt格式输出)：

    1.  整体表现总结 (Overall Performance Summary)
    2.  知识亮点 (Strengths)
    3.  待提升点 (Weaknesses)
    4.  学习建议 (Actionable Advice)
    """

    try:
        sys.stderr.write("正在通过火山方舟向豆包大模型发送请求...\n")
        response = client.chat.completions.create(
            model=ENDPOINT_ID,
            messages=[
                {"role": "system", "content": "你是一名资深的C语言编程辅导老师..."},
                {"role": "user", "content": prompt_content}
            ],
            temperature=0.7
        )
        return response.choices[0].message.content

    except Exception as e:
        return f"\n[方舟API调用失败]: 请检查 Endpoint ID 是否正确、账户是否正常。错误详情: {e}"


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("用法: python3 analyze_exam.py <json_log_file>")
        sys.exit(1)

    json_file = sys.argv[1]

    try:
        with open(json_file, 'r', encoding='utf-8') as f:
            data = json.load(f)
        
        if not data:
            print("\n[分析报告]: 记录为空。")
        else:
            report = analyze_performance(data)
            print("\n" + "="*50)
            print(" C语言学习智能评估报告 (大模型生成-仅供参考)")
            print("="*50)
            print(report)
            print("="*50)
    except FileNotFoundError:
        print(f"错误：找不到日志文件 {json_file}")
    except json.JSONDecodeError:
        print(f"错误：日志文件 {json_file} 格式不正确。")